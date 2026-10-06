#include "FloatingBar.h"
#include "../core/ThemeManager.h"

#include <QRegularExpression>
#include <QtMath>

// 解析主题里的颜色：支持 rgb(r, g, b)、rgba(r, g, b, a)（a 可以是 0~1 或 0~255）、#RRGGBB、#AARRGGBB 和颜色名
static QColor parseThemeColor(const QString& text)
{
    static const QRegularExpression rgbaExpression("^rgba?\\(\\s*([\\d.]+)\\s*,\\s*([\\d.]+)\\s*,\\s*([\\d.]+)\\s*(?:,\\s*([\\d.]+)\\s*)?\\)$");
    const QString value = text.trimmed();
    if (const QRegularExpressionMatch match = rgbaExpression.match(value); match.hasMatch()) {
        qreal alpha = match.captured(4).isEmpty() ? 255 : match.captured(4).toDouble();
        if (alpha <= 1.0) alpha *= 255;
        return QColor(match.captured(1).toInt(), match.captured(2).toInt(), match.captured(3).toInt(), qBound(0, qRound(alpha), 255));
    }
    return QColor::fromString(value);
}

FloatingBarClass::FloatingBarClass(QWidget* parent)
    : QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground); // 设置窗口背景透明

    Bar = new QLabel(this);
    Bar->setAlignment(Qt::AlignCenter);
    Bar->show();

    m_shadowEffect = new QGraphicsDropShadowEffect(Bar);
    m_shadowEffect->setEnabled(false);
    Bar->setGraphicsEffect(m_shadowEffect);
}

FloatingBarClass::~FloatingBarClass()
{}

void FloatingBarClass::applyTheme(unsigned radius, unsigned height)
{
    ThemeManager& theme = ThemeManager::instance();
    Bar->setStyleSheet(theme.style("FloatingBar", {
        { "floating_bar_radius", QString::number(radius) + "px" },
        { "floating_bar_height", QString::number(height) + "px" },
        }));

    // QSS 不支持阴影，由主题变量决定是否画阴影
    const QColor shadowColor = parseThemeColor(theme.variable("floating_bar_shadow_color"));
    if (shadowColor.isValid() && shadowColor.alpha() > 0) {
        m_shadowEffect->setColor(shadowColor);
        m_shadowEffect->setBlurRadius(theme.variable("floating_bar_shadow_blur").toDouble());
        m_shadowEffect->setOffset(theme.variable("floating_bar_shadow_offset_x").toDouble(), theme.variable("floating_bar_shadow_offset_y").toDouble());
        m_shadowEffect->setEnabled(true);
    }
    else {
        m_shadowEffect->setEnabled(false);
    }
}

QMargins FloatingBarClass::shadowMargins(FloatingBarPosition position) const
{
    if (!m_shadowEffect->isEnabled())
        return QMargins();
    // 阴影向四周扩散模糊半径，再按偏移移动；悬浮条贴着屏幕顶边，上方不留
    const int blur = qCeil(m_shadowEffect->blurRadius());
    const int dx = qRound(m_shadowEffect->xOffset());
    const int dy = qRound(m_shadowEffect->yOffset());
    QMargins margins(qMax(0, blur - dx), 0, qMax(0, blur + dx), qMax(0, blur + dy));
    if (position == FloatingBarPosition::TopLeft)
        margins.setLeft(0);
    if (position == FloatingBarPosition::TopRight)
        margins.setRight(0);
    return margins;
}
