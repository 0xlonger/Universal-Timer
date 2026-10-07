#include "FloatingBar.h"
#include "../core/ThemeManager.h"

#include <QRegularExpression>
#include <QtMath>
#include <QCursor>
#include <QGuiApplication>
#include <QWindow>

// 鼠标移入时的不透明度和渐变时长，与 ClassIsland 一致
static constexpr double MOUSE_IN_FADED_OPACITY = 0.05;
static constexpr int OPACITY_ANIMATION_DURATION = 100;

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

FloatingBarClass::FloatingBarClass(ConfigManager& cfg, QWidget* parent)
    : QWidget(parent), config(cfg)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground); // 设置窗口背景透明

    Bar = new QLabel(this);
    Bar->setAlignment(Qt::AlignCenter);
    Bar->show();

    m_shadowEffect = new QGraphicsDropShadowEffect(Bar);
    m_shadowEffect->setEnabled(false);
    Bar->setGraphicsEffect(m_shadowEffect);

    m_opacityAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_opacityAnimation->setDuration(OPACITY_ANIMATION_DURATION);

    m_mouseTimer = new QTimer(this);
    m_mouseTimer->setInterval(50);
    connect(m_mouseTimer, &QTimer::timeout, this, &FloatingBarClass::updateMouseStatus);
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

QMargins FloatingBarClass::shadowMargins(FloatingBarPosition position, unsigned topMargin) const
{
    if (!m_shadowEffect->isEnabled())
        return QMargins();
    // 阴影向四周扩散模糊半径，再按偏移移动；上方最多留到屏幕顶边（贴着顶边时上方不留）
    const int blur = qCeil(m_shadowEffect->blurRadius());
    const int dx = qRound(m_shadowEffect->xOffset());
    const int dy = qRound(m_shadowEffect->yOffset());
    QMargins margins(qMax(0, blur - dx), qMin(qMax(0, blur - dy), int(topMargin)), qMax(0, blur + dx), qMax(0, blur + dy));
    if (position == FloatingBarPosition::TopLeft)
        margins.setLeft(0);
    if (position == FloatingBarPosition::TopRight)
        margins.setRight(0);
    return margins;
}

void FloatingBarClass::updateWindowFlags()
{
    Qt::WindowFlags flags = (config.floating_bar.floating_bar_on_top ? Qt::WindowStaysOnTopHint : Qt::WindowStaysOnBottomHint) | Qt::FramelessWindowHint | Qt::Tool;
    if (config.floating_bar.is_mouse_click_through_enabled)
        flags |= Qt::WindowTransparentForInput; // 鼠标点击直接穿过悬浮条
    if (flags == this->windowFlags())
        return;
    const bool visible = this->isVisible();
    this->setWindowFlags(flags);
    if (visible)
        this->show(); // 修改窗口标志会把窗口隐藏
}

void FloatingBarClass::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    m_isClickThroughPending = true;
}

void FloatingBarClass::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    // 第一次画出来时窗口已经真正显示了（已被窗口管理器接管），这时再设置一次点击穿透
    if (m_isClickThroughPending) {
        m_isClickThroughPending = false;
        QTimer::singleShot(0, this, &FloatingBarClass::reapplyClickThrough);
    }
}

void FloatingBarClass::reapplyClickThrough()
{
    // Linux 上 XFCE 的 xfwm4 会给窗口套一层外框，它只在接管窗口之后收到点击穿透（输入区域）的变化时才会让外框也跟着穿透，
    // 而 Qt 是在窗口隐藏时设置的，所以窗口显示后在这里关一下再开一下。Windows 没有这个问题
    if (QGuiApplication::platformName() != "xcb" || !config.floating_bar.is_mouse_click_through_enabled || !this->isVisible() || !this->windowHandle())
        return;
    this->windowHandle()->setFlag(Qt::WindowTransparentForInput, false);
    this->windowHandle()->setFlag(Qt::WindowTransparentForInput, true);
}

void FloatingBarClass::updateOpacity()
{
    const bool fading = config.floating_bar.is_mouse_in_fading_enabled;
    if (fading && !m_mouseTimer->isActive())
        m_mouseTimer->start();
    else if (!fading && m_mouseTimer->isActive())
        m_mouseTimer->stop();
    if (!fading)
        m_isMouseIn = false;

    double opacity = qBound(0u, config.floating_bar.floating_bar_opacity, 100u) / 100.0;
    if (m_isMouseIn)
        opacity = qMin(opacity, MOUSE_IN_FADED_OPACITY);

    m_opacityAnimation->stop();
    m_opacityAnimation->setStartValue(this->windowOpacity());
    m_opacityAnimation->setEndValue(opacity);
    m_opacityAnimation->start();
}

void FloatingBarClass::updateMouseStatus()
{
    // 开了点击穿透后窗口收不到鼠标事件，所以和 ClassIsland 一样直接查询鼠标位置；触摸也会移动鼠标位置，处理方式相同
    const bool mouseIn = this->isVisible() && QRect(Bar->mapToGlobal(QPoint(0, 0)), Bar->size()).contains(QCursor::pos());
    if (mouseIn != m_isMouseIn) {
        m_isMouseIn = mouseIn;
        updateOpacity();
    }
}
