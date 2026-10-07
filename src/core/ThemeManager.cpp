#include "core/ThemeManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QStyleHints>
#include <QDebug>
#include <QCryptographicHash>

static const QString THEMES_DIR = "themes";
static const QString BUILT_IN_THEMES_DIR = ":/themes";
static const QString PRISTINE_THEMES_DIR = "themes/.builtin"; // 导出时另存的原始副本

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // 系统切换深浅色时，用了 @var-dark 的主题跟着变
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] {
        if (!m_theme.darkVars.isEmpty() || !m_default.darkVars.isEmpty())
            emit changed();
        });
#endif
}

QString ThemeManager::read(const QString& id)
{
    for (const QString& dir : { THEMES_DIR, BUILT_IN_THEMES_DIR }) {
        QFile file(dir + "/" + id + ".qss");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString::fromUtf8(file.readAll());
    }
    return QString();
}

ThemeManager::Theme ThemeManager::parse(const QString& text)
{
    Theme theme;
    // 去掉注释，注释里的 @section 等不算
    QString content = text;
    content.remove(QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption));

    static const QRegularExpression varExpression("^@(var|var-dark)\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*(.*?)\\s*;?\\s*$");
    QString section;
    for (const QString& rawLine : content.split('\n')) {
        const QString line = rawLine.trimmed();
        if (line.startsWith("@name ")) {
            theme.name = line.mid(6).trimmed();
            if (theme.name.endsWith(';')) theme.name.chop(1);
            theme.name = theme.name.trimmed();
        }
        else if (line.startsWith("@section ")) {
            section = line.mid(9).trimmed();
            theme.sections.insert(section, QString());
        }
        else if (const QRegularExpressionMatch match = varExpression.match(line); match.hasMatch()) {
            (match.captured(1) == "var" ? theme.vars : theme.darkVars).insert(match.captured(2), match.captured(3));
        }
        else if (!section.isEmpty()) {
            theme.sections[section] += rawLine + '\n';
        }
    }
    return theme;
}

void ThemeManager::load(const QString& id)
{
    m_currentId = id.isEmpty() ? "default" : id;
    m_builtInDefault = parse([] {
        QFile file(BUILT_IN_THEMES_DIR + "/default.qss");
        return file.open(QIODevice::ReadOnly | QIODevice::Text) ? QString::fromUtf8(file.readAll()) : QString();
        }());
    m_default = parse(read("default"));
    const QString text = read(m_currentId);
    if (text.isEmpty()) {
        qWarning() << "找不到主题" << m_currentId << "，使用默认主题";
        m_theme = m_default;
    }
    else {
        m_theme = parse(text);
    }
    qInfo() << "已加载主题" << m_currentId;
    emit changed();
}

QList<ThemeManager::ThemeInfo> ThemeManager::themes() const
{
    QStringList ids;
    for (const QString& dir : { BUILT_IN_THEMES_DIR, THEMES_DIR })
        for (const QFileInfo& info : QDir(dir).entryInfoList({ "*.qss" }, QDir::Files, QDir::Name))
            if (!ids.contains(info.completeBaseName()))
                ids.append(info.completeBaseName());
    // “默认”放在最前面
    if (ids.removeOne("default"))
        ids.prepend("default");

    QList<ThemeInfo> result;
    for (const QString& id : ids) {
        const QString name = parse(read(id)).name;
        result.append({ id, name.isEmpty() ? id : name });
    }
    return result;
}

bool ThemeManager::isDark() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#else
    return false;
#endif
}

QString ThemeManager::lookupVariable(const QString& name) const
{
    // 当前主题优先，然后是“默认”主题和内置“默认”主题（区段可能来自它们）
    for (const Theme* theme : { &m_theme, &m_default, &m_builtInDefault }) {
        if (isDark() && theme->darkVars.contains(name))
            return theme->darkVars.value(name);
        if (theme->vars.contains(name))
            return theme->vars.value(name);
    }
    return QString();
}

QString ThemeManager::variable(const QString& name) const
{
    return lookupVariable(name);
}

QString ThemeManager::style(const QString& section, const QHash<QString, QString>& vars) const
{
    QString text;
    bool found = false;
    for (const Theme* theme : { &m_theme, &m_default, &m_builtInDefault }) {
        if (theme->sections.contains(section)) {
            text = theme->sections.value(section);
            found = true;
            break;
        }
    }
    if (!found)
        qWarning() << "主题里没有区段" << section;

    // 替换 @变量：先找程序提供的变量，再找主题变量；变量的值里也可以引用别的变量（最多展开几层）
    static const QRegularExpression variableExpression("@([A-Za-z_][A-Za-z0-9_]*)");
    for (int depth = 0; depth < 8 && text.contains('@'); depth++) {
        QString result;
        qsizetype last = 0;
        bool replaced = false;
        for (QRegularExpressionMatchIterator it = variableExpression.globalMatch(text); it.hasNext();) {
            const QRegularExpressionMatch match = it.next();
            const QString name = match.captured(1);
            QString value = vars.contains(name) ? vars.value(name) : lookupVariable(name);
            if (value.isNull()) {
                qWarning() << "主题区段" << section << "里的变量" << name << "没有定义";
                continue;
            }
            result += text.mid(last, match.capturedStart() - last) + value;
            last = match.capturedEnd();
            replaced = true;
        }
        if (!replaced)
            break;
        text = result + text.mid(last);
    }
    return text;
}

// 规范化主题文件内容：去掉 \r，Windows 上检出的文件是 CRLF
static QByteArray normalizedThemeContent(const QByteArray& content)
{
    QByteArray result = content;
    result.replace("\r", "");
    return result;
}

static QByteArray readFileContent(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

static bool writeFileContent(const QString& path, const QByteArray& content)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(content) == content.size();
}

void ThemeManager::exportBuiltInThemes()
{
    // 以前版本导出的内置主题（规范化后的 SHA-256）。用户的文件和它们一样，说明导出后没改过，可以换成新版
    static const QHash<QString, QStringList> OLD_BUILT_IN_THEMES = {
        { "default.qss", { "1f388661a5304adc2f3c2d42ff4b7746691bd809062e0de867ce383af5c3866b" } },
        { "win11.qss", { "122b7064e37746f9168eedb164946f16296e26d78e7daf7ecd426e5ac30be873" } },
    };
    if (!QDir().mkpath(PRISTINE_THEMES_DIR)) {
        qWarning() << "无法创建主题文件夹" << THEMES_DIR;
        return;
    }
    for (const QFileInfo& info : QDir(BUILT_IN_THEMES_DIR).entryInfoList({ "*.qss" }, QDir::Files)) {
        const QByteArray builtIn = readFileContent(info.filePath());
        const QString targetPath = THEMES_DIR + "/" + info.fileName();
        const QString pristinePath = PRISTINE_THEMES_DIR + "/" + info.fileName();
        bool update = !QFile::exists(targetPath);
        if (!update) {
            // 用户没改过（和导出时另存的原始副本一样，或者和以前版本的内置主题一样）才换成新版，改过的保留
            const QByteArray current = normalizedThemeContent(readFileContent(targetPath));
            if (current == normalizedThemeContent(builtIn))
                update = false;
            else if (QFile::exists(pristinePath))
                update = current == normalizedThemeContent(readFileContent(pristinePath));
            else
                update = OLD_BUILT_IN_THEMES.value(info.fileName()).contains(QString::fromLatin1(QCryptographicHash::hash(current, QCryptographicHash::Sha256).toHex()));
            if (!update && current != normalizedThemeContent(builtIn))
                qInfo() << "主题文件" << targetPath << "改过，保留";
        }
        if (update) {
            if (writeFileContent(targetPath, builtIn))
                qInfo() << "已导出内置主题" << QFileInfo(targetPath).absoluteFilePath();
            else
                qWarning() << "无法导出内置主题" << targetPath;
        }
        if (normalizedThemeContent(readFileContent(pristinePath)) != normalizedThemeContent(builtIn))
            writeFileContent(pristinePath, builtIn); // 原始副本，下次启动时用来判断用户有没有改过
    }
}
