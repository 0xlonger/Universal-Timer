#pragma once

// 主题管理：读取 themes/ 文件夹（和 config.ini 在同一目录）或内置的 .qss 主题文件，按区段提供样式表
//
// 主题文件就是普通的 QSS，另外支持这几种写法（见 assets/themes/default.qss 开头的说明）：
//   @name 主题名称;                   在设置里显示的名称
//   @var 变量名 = 值;                 主题自己的变量，在样式里用 @变量名 引用
//   @var-dark 变量名 = 值;            系统是深色模式时这个变量改用这个值（跟随系统深浅色）
//   @section 区段名                   下面的样式用于界面的哪个部分，直到下一个 @section
// 主题里没写的区段会使用“默认”主题的；程序还会提供一些按屏幕大小或设置算出来的变量（如 @floating_bar_radius）

#include <QObject>
#include <QHash>
#include <QList>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    struct ThemeInfo {
        QString id; // 文件名（不含 .qss）
        QString name; // 显示名称（@name），没写时等于 id
    };

    static ThemeManager& instance();

    void load(const QString& id); // 读取（或重新读取）主题文件，读取后发出 changed()
    QString currentId() const { return m_currentId; }
    QList<ThemeInfo> themes() const; // 所有可选的主题：内置主题和 themes/ 文件夹里的主题

    // 取一个区段的样式表；vars 是程序提供的变量（变量名不带 @）
    QString style(const QString& section, const QHash<QString, QString>& vars = {}) const;
    QString variable(const QString& name) const; // 主题变量的值（已按系统深浅色选好），没有时为空

    static void exportBuiltInThemes(); // 把内置主题导出到 themes/ 文件夹，方便照着修改；已导出但没改过的会换成新版

Q_SIGNALS:
    void changed(); // 主题重新读取了，或系统深浅色变了，需要重新设置样式表

private:
    explicit ThemeManager(QObject* parent = nullptr);

    struct Theme {
        QString name;
        QHash<QString, QString> vars;
        QHash<QString, QString> darkVars;
        QHash<QString, QString> sections;
    };

    static Theme parse(const QString& text);
    static QString read(const QString& id); // 先找 themes/ 文件夹，再找内置主题
    bool isDark() const;
    QString lookupVariable(const QString& name) const;

    QString m_currentId;
    Theme m_theme; // 当前主题
    Theme m_default; // “默认”主题（themes/default.qss 或内置），当前主题没写的区段用它
    Theme m_builtInDefault; // 内置的“默认”主题，保证所有区段都有
};
