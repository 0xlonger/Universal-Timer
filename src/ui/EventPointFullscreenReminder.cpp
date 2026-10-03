/*
    *  EventPointFullscreenReminder.cpp
    *  本文件由 Claude Opus 5.5 Extra 生成
*/
#include "EventPointFullscreenReminder.h"

#include <QApplication>
#include <QEasingCurve>
#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QScreen>
#include <QSoundEffect>
#include <QVariantAnimation>
#include <QtMath>

#include <algorithm>
#include <cmath>

namespace {

constexpr qreal PULSE_START = 2200;  // 第一次闪烁
constexpr qreal PULSE_GAP = 1000;    // 与倒计时全屏提醒的方块一致：亮 500ms + 灭 500ms
constexpr qreal FLASH_ON = 500;
constexpr qreal CAPSULE_BORDER = 5;  // EventPointReminder 的红边宽度

const QColor RED(255, 0, 0);

const QEasingCurve OUT_CUBIC(QEasingCurve::OutCubic);
const QEasingCurve IN_CUBIC(QEasingCurve::InCubic);
const QEasingCurve IN_OUT_CUBIC(QEasingCurve::InOutCubic);
const QEasingCurve OUT_BACK(QEasingCurve::OutBack);

qreal clamp01(qreal x) { return std::clamp<qreal>(x, 0, 1); }
qreal progress(qreal t, qreal from, qreal to) { return clamp01((t - from) / (to - from)); }
qreal lerp(qreal a, qreal b, qreal p) { return a + (b - a) * p; }
qreal ease(const QEasingCurve& curve, qreal p) { return curve.valueForProgress(p); }
QColor rgba(int r, int g, int b, qreal a) { return QColor(r, g, b, qRound(255 * clamp01(a))); }

// 数字 / 英文：与 ReminderPage 的字体顺序一致，后面补上 Windows 自带的 Bahnschrift
QFont dinFont(qreal pixelSize, QFont::Weight weight, qreal spacing = 0)
{
    QFont font;
    font.setFamilies({ "DIN1451", "Bahnschrift", "Barlow Condensed", "Arial Narrow" });
    font.setPixelSize(qMax(1, qRound(pixelSize)));
    font.setWeight(weight);
    if (spacing != 0) font.setLetterSpacing(QFont::AbsoluteSpacing, spacing);
    return font;
}

// 中文：字魂59号-创粗黑 → 微软雅黑
QFont cnFont(qreal pixelSize, QFont::Weight weight)
{
    QFont font;
    font.setFamilies({ "zihun59hao-chuangcuhei", "Microsoft YaHei UI", "Microsoft YaHei", "Noto Sans CJK SC", "PingFang SC" });
    font.setPixelSize(qMax(1, qRound(pixelSize)));
    font.setWeight(weight);
    return font;
}

// 文字宽度（去掉最后一个字后面多出来的字间距）
qreal textWidth(const QFont& font, const QString& text)
{
    const qreal spacing = font.letterSpacingType() == QFont::AbsoluteSpacing ? font.letterSpacing() : 0;
    return QFontMetricsF(font).horizontalAdvance(text) - spacing;
}

// 把文字放在 left 处，字形的视觉中心落在 centerY；返回墨迹包围盒，baseline 输出 drawText 用的基线点
QRectF placeText(const QFont& font, const QString& text, qreal left, qreal centerY, QPointF* baseline)
{
    const QRectF ink = QFontMetricsF(font).tightBoundingRect(text);
    const qreal baseY = centerY - (ink.top() + ink.bottom()) / 2;
    *baseline = QPointF(left, baseY);
    return QRectF(left, baseY + ink.top(), textWidth(font, text), ink.height());
}

// 红块擦除：in 0→1 显现（色块伸出 → 色块收回露出文字），out 0→1 消失（反向）
template <typename Draw>
void wipe(QPainter& painter, const QRectF& box, qreal in, qreal out, const QColor& block, Draw draw)
{
    if (in <= 0 || out >= 1) return;
    const qreal x = box.left(), y = box.top(), w = box.width(), h = box.height();
    if (out > 0) {
        if (out < 0.5) {
            const qreal q = ease(OUT_CUBIC, out / 0.5);
            painter.save();
            painter.setClipRect(QRectF(x + w * q, y - h, w * (1 - q) + 2, h * 3));
            draw();
            painter.restore();
            painter.fillRect(QRectF(x, y, w * q, h), block);
        } else {
            const qreal q = ease(IN_OUT_CUBIC, (out - 0.5) / 0.5);
            painter.fillRect(QRectF(x + w * q, y, w * (1 - q), h), block);
        }
        return;
    }
    if (in < 0.5) {
        painter.fillRect(QRectF(x, y, w * ease(OUT_CUBIC, in / 0.5), h), block);
    } else if (in < 1) {
        const qreal q = ease(IN_OUT_CUBIC, (in - 0.5) / 0.5);
        painter.save();
        painter.setClipRect(QRectF(x - 2, y - h, w * q + 2, h * 3));
        draw();
        painter.restore();
        painter.fillRect(QRectF(x + w * q, y, w * (1 - q), h), block);
    } else {
        draw();
    }
}

// V 形门板：内侧是接缝（上下两端在 seamEnd，中间凸到 seamMid），向外延伸 width（负数向左）
QPainterPath chevronBand(qreal seamEnd, qreal seamMid, qreal width, qreal offset, qreal height)
{
    const qreal cy = height / 2;
    QPainterPath path;
    path.moveTo(seamEnd + offset, 0);
    path.lineTo(seamMid + offset, cy);
    path.lineTo(seamEnd + offset, height);
    path.lineTo(seamEnd + width + offset, height);
    path.lineTo(seamMid + width + offset, cy);
    path.lineTo(seamEnd + width + offset, 0);
    path.closeSubpath();
    return path;
}

// 斜纹警示带
void drawStripes(QPainter& painter, qreal x0, qreal x1, qreal y, qreal h, qreal offset, qreal unit)
{
    const qreal period = 0.032 * unit, stripe = period / 2;
    const qreal shift = std::fmod(std::fmod(offset, period) + period, period);
    QPainterPath path;
    for (qreal x = x0 - h - period + shift; x < x1 + h; x += period) {
        path.moveTo(x, y + h);
        path.lineTo(x + h, y);
        path.lineTo(x + h + stripe, y);
        path.lineTo(x + stripe, y + h);
        path.closeSubpath();
    }
    painter.save();
    painter.setClipRect(QRectF(x0, y, x1 - x0, h));
    painter.fillPath(path, rgba(255, 0, 0, 0.6));
    painter.restore();
}

// 四周信息区：上下警示带与角落三角之间的范围
struct HudFrame {
    qreal margin, band, left, right;
};
HudFrame hudFrame(qreal unit, qreal width)
{
    const qreal margin = 0.05 * unit, corner = 0.1 * unit;
    const qreal left = margin + corner + 0.03 * unit;
    return { margin, 0.016 * unit, left, width - left };
}

} // namespace

EventPointFullscreenReminder::EventPointFullscreenReminder(EventPoint& eventPoint,
                                                           const QList<EventPoint>& eventPointList,
                                                           const QRect& capsuleGeometry,
                                                           int flashTimes,
                                                           QWidget* parent)
    : QWidget(parent),
      m_eventPoint(eventPoint),
      m_eventPointList(eventPointList),
      m_capsuleGeometry(capsuleGeometry),
      m_flashTimes(qMax(0, flashTimes))
{
    // 与 FullscreenPagesManager 相同的窗口属性
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_ShowWithoutActivating);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setGeometry(QApplication::primaryScreen()->geometry());

    m_exit = m_flashTimes > 0 ? PULSE_START + (m_flashTimes - 1) * PULSE_GAP + FLASH_ON + 800 : PULSE_START + 2500;
    m_end = m_exit + 1000;

    //m_countdownSound = new QSoundEffect(this);
    //m_countdownSound->setSource(QUrl::fromLocalFile("./sounds/countdown.wav"));

    // 整个动画只有这一条时间轴，每个图层在 paintEvent 里按 m_t 算自己的进度
    m_timeline = new QVariantAnimation(this);
    m_timeline->setStartValue(0.0);
    m_timeline->setEndValue(m_end);
    m_timeline->setDuration(qRound(m_end));
    connect(m_timeline, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        const qreal previous = m_t;
        m_t = value.toReal();
        for (int i = 0; i < m_flashTimes; i++) {
            const qreal pulse = PULSE_START + i * PULSE_GAP;
            // 只在正常经过时播放，点击跳到收尾时不会连响
            //if (previous < pulse && m_t >= pulse && m_t - pulse < 200) m_countdownSound->play();
        }
        this->update();
        });
    connect(m_timeline, &QVariantAnimation::finished, this, [this] {
        m_eventPoint.setShowing(false);
        emit finished();
        this->close();
        });
}

void EventPointFullscreenReminder::start()
{
    m_t = 0;
    this->show();
    this->raise();
    m_timeline->start();
}

void EventPointFullscreenReminder::seek(qreal ms)
{
    m_timeline->stop();
    m_t = std::clamp<qreal>(ms, 0, m_end);
    this->update();
}

qreal EventPointFullscreenReminder::totalDuration() const
{
    return m_end;
}

void EventPointFullscreenReminder::mousePressEvent(QMouseEvent* event)
{
    if (m_timeline->state() == QAbstractAnimation::Running && m_t < m_exit) {
        m_timeline->setCurrentTime(qRound(m_exit));
    }
    event->accept();
}

void EventPointFullscreenReminder::updateMetrics()
{
    m_width = qMax(1, this->width());
    m_height = qMax(1, this->height());
    m_unit = qMin(m_height, m_width * 9 / 16);
    m_px = m_unit / 1080;
    m_cx = m_width / 2;
    m_cy = m_height / 2;
}

EventPointFullscreenReminder::Layout EventPointFullscreenReminder::layout() const
{
    Layout lay;
    lay.name = m_eventPoint.name().trimmed();
    if (lay.name.isEmpty()) lay.name = tr("事件点");

    // 名称默认 0.2 倍屏高（与倒计时页标题一致），圆环跟着名称变大，太长时缩小字号
    const qreal baseSize = 0.2 * m_unit;
    const qreal nameWidth = textWidth(cnFont(baseSize, QFont::Black), lay.name);
    const qreal padding = 0.085 * m_unit, minRadius = 0.3 * m_unit, maxRadius = 0.38 * m_unit;
    lay.radius = std::clamp(nameWidth / 2 + padding, minRadius, maxRadius);
    lay.nameSize = nameWidth / 2 + padding > maxRadius ? baseSize * 2 * (maxRadius - padding) / nameWidth : baseSize;
    return lay;
}

QRectF EventPointFullscreenReminder::capsuleRect() const
{
    if (!m_capsuleGeometry.isNull()) {
        return QRectF(m_capsuleGeometry.translated(-this->pos()));
    }
    // 与 EventPointReminder 一致：y = 10% 屏高，高 = 5% 屏宽
    const qreal h = 0.05 * m_width, w = h * 0.6;
    return QRectF(m_cx - w / 2, 0.1 * m_height, w, h);
}

qreal EventPointFullscreenReminder::pulseEnvelope(qreal t) const
{
    qreal value = 0;
    for (int i = 0; i < m_flashTimes; i++) {
        const qreal age = t - (PULSE_START + i * PULSE_GAP);
        if (age >= 0 && age < 900) value = qMax(value, std::exp(-age / 200));
    }
    return value;
}

bool EventPointFullscreenReminder::isFlashing(qreal t) const
{
    for (int i = 0; i < m_flashTimes; i++) {
        const qreal age = t - (PULSE_START + i * PULSE_GAP);
        if (age >= 0 && age < FLASH_ON) return true;
    }
    return false;
}

void EventPointFullscreenReminder::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    updateMetrics();
    const qreal t = m_t;
    if (t >= m_end) return;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 收尾淡出，效果等同 FullscreenPagesManager 的 FadeOutAnimation（windowOpacity 1 → 0，InCubic）
    painter.setOpacity(1 - ease(IN_CUBIC, progress(t, m_exit, m_end)));
    painter.fillRect(this->rect(), rgba(0, 0, 0, 0.75 * ease(OUT_CUBIC, progress(t, 120, 650))));

    const Layout lay = layout();
    drawCapsule(painter, t);
    drawBands(painter, t);
    drawCorners(painter, t);
    drawShockwaves(painter, t, lay.radius);
    drawRing(painter, t, lay.radius);
    drawTexts(painter, t, lay);
    drawHud(painter, t);
    drawStrike(painter, t);
    drawDoors(painter, t);
}

// 0 ~ 300：胶囊收窄成红线（同 EventPointReminder 的 m_fadeOutAnimation1）
void EventPointFullscreenReminder::drawCapsule(QPainter& painter, qreal t) const
{
    if (t >= 300) return;
    const QRectF capsule = capsuleRect();
    const qreal w = lerp(capsule.width(), CAPSULE_BORDER, ease(OUT_CUBIC, progress(t, 0, 300)));
    const QRectF r(capsule.center().x() - w / 2, capsule.top(), w, capsule.height());
    painter.fillRect(r, rgba(0, 0, 0, 0.75));
    const qreal border = qMin(CAPSULE_BORDER, w / 2);
    painter.fillRect(QRectF(r.left(), r.top(), border, r.height()), RED);
    painter.fillRect(QRectF(r.right() - border, r.top(), border, r.height()), RED);
}

// 300 ~ 650：红线拉满屏高
void EventPointFullscreenReminder::drawStrike(QPainter& painter, qreal t) const
{
    if (t < 300 || t >= 900) return;
    const QRectF capsule = capsuleRect();
    const qreal p = ease(IN_OUT_CUBIC, progress(t, 300, 650));
    const qreal w = lerp(CAPSULE_BORDER, qMax<qreal>(CAPSULE_BORDER, 8 * m_px), p);
    const qreal x = lerp(capsule.center().x(), m_cx, p);
    const qreal top = lerp(capsule.top(), 0, p), bottom = lerp(capsule.bottom(), m_height, p);
    painter.fillRect(QRectF(x - w / 2, top, w, bottom - top), RED);
}

// 650 ~ 1020 合拢成满屏红色，1020 ~ 2020 向两侧分开
void EventPointFullscreenReminder::drawDoors(QPainter& painter, qreal t) const
{
    const qreal expand = ease(IN_CUBIC, progress(t, 650, 900));
    const qreal open = ease(OUT_CUBIC, progress(t, 1020, 2020));
    if (expand <= 0 || open >= 1) return;

    const qreal depth = 0.1 * m_unit * expand;  // V 形深度随展开长出来，起点是一条直线
    const qreal seamEnd = m_cx - depth / 2, seamMid = m_cx + depth / 2;
    const qreal leftWidth = expand * (seamMid + 6), rightWidth = expand * (m_width - seamEnd + 6);
    const qreal leftOffset = -(seamMid + 6) * open, rightOffset = (m_width - seamEnd + 6) * open;
    const qreal edge = 0.009 * m_unit * expand;

    const QFont font = dinFont(0.34 * m_unit, QFont::Bold);
    const QString time = m_eventPoint.time().toString("HH:mm");
    QPointF base;
    const QRectF box = placeText(font, time, 0, m_cy, &base);
    const qreal textLeft = m_cx - box.width() / 2;

    struct Door {
        QPainterPath body, edge;
        qreal offset;
    };
    const Door doors[] = {
        { chevronBand(seamEnd, seamMid, -leftWidth, leftOffset, m_height), chevronBand(seamEnd, seamMid, -edge, leftOffset, m_height), leftOffset },
        { chevronBand(seamEnd, seamMid, rightWidth, rightOffset, m_height), chevronBand(seamEnd, seamMid, edge, rightOffset, m_height), rightOffset },
    };
    for (const Door& door : doors) {
        painter.fillPath(door.body, RED);
        // 门上的黑色时间：按门的形状裁剪后再画，两扇门各画一次，字就被接缝切开并跟着门走
        painter.save();
        painter.setClipPath(door.body);
        painter.setFont(font);
        painter.setPen(rgba(0, 0, 0, 0.86));
        painter.drawText(QPointF(textLeft + door.offset, base.y()), time);
        painter.restore();
        painter.fillPath(door.edge, QColor(150, 0, 0));
    }
}

// 圆环（取自图标：四段四分之一圆环 + 表盘刻度）
void EventPointFullscreenReminder::drawRing(QPainter& painter, qreal t, qreal radius) const
{
    const qreal reveal = ease(OUT_CUBIC, progress(t, 1080, 2000));
    if (reveal <= 0) return;
    const qreal leave = ease(IN_CUBIC, progress(t, m_exit, m_exit + 380));
    const qreal pulse = pulseEnvelope(t);
    const QPointF center(m_cx, m_cy);

    // 光晕：径向渐变，不需要模糊
    const qreal glowAlpha = reveal * (1 - leave);
    QRadialGradient glow(center, radius * 1.6);
    glow.setColorAt(0, rgba(255, 0, 0, (0.12 + 0.12 * pulse) * glowAlpha));
    glow.setColorAt(0.55, rgba(255, 0, 0, (0.05 + 0.06 * pulse) * glowAlpha));
    glow.setColorAt(1, rgba(255, 0, 0, 0));
    painter.fillRect(QRectF(m_cx - radius * 1.6, m_cy - radius * 1.6, radius * 3.2, radius * 3.2), glow);

    // 60 格刻度：先逐格出现，再顺时针扫红
    const qreal shown = progress(t, 1150, 1650);
    const qreal swept = ease(IN_OUT_CUBIC, progress(t, 1550, 2200));
    for (int i = 0; i < 60; i++) {
        if (i / 60.0 >= shown) break;
        const bool major = i % 5 == 0;
        const qreal angle = qDegreesToRadians(i * 6.0 - 90);  // 12 点钟为起点，顺时针
        const qreal r1 = radius + 0.034 * m_unit, r2 = radius + (major ? 0.064 : 0.05) * m_unit;
        const QColor color = i / 60.0 < swept
            ? rgba(255, 0, 0, (0.62 + 0.38 * pulse) * (1 - leave))
            : rgba(255, 255, 255, 0.3 * (1 - leave));
        painter.setPen(QPen(color, (major ? 0.0045 : 0.0025) * m_unit, Qt::SolidLine, Qt::FlatCap));
        painter.drawLine(QPointF(m_cx + std::cos(angle) * r1, m_cy + std::sin(angle) * r1),
                         QPointF(m_cx + std::cos(angle) * r2, m_cy + std::sin(angle) * r2));
    }

    // 内圈虚线，缓慢转动；Qt 的虚线长度和偏移都以笔宽为单位
    const qreal dashAlpha = progress(t, 1300, 1800) * (1 - leave);
    if (dashAlpha > 0) {
        const qreal w = qMax<qreal>(1, 1.5 * m_px);
        QPen pen(rgba(255, 255, 255, 0.28 * dashAlpha), w, Qt::CustomDashLine, Qt::FlatCap);
        pen.setDashPattern({ 0.014 * m_unit / w, 0.01 * m_unit / w });
        pen.setDashOffset(t * 0.012 * m_px / w);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, radius - 0.05 * m_unit, radius - 0.05 * m_unit);
    }

    // 四段粗弧：错开生长，整体从 -90° 转入；脉冲时加粗
    static const QColor arcColors[4] = {
        QColor(255, 0, 0),            // 右上
        QColor(150, 0, 0),            // 右下
        QColor(255, 0, 0, 128),       // 左下
        QColor(255, 255, 255, 235),   // 左上
    };
    const qreal rotation = -90 * (1 - reveal) + 50 * leave;
    const qreal gap = 6;
    const QRectF ringRect(m_cx - radius, m_cy - radius, radius * 2, radius * 2);
    for (int i = 0; i < 4; i++) {
        const qreal grow = ease(OUT_CUBIC, progress(t, 1080 + i * 90, 1080 + i * 90 + 720)) * (1 - leave);
        if (grow <= 0) continue;
        const qreal start = i * 90 + gap / 2 + rotation;  // 12 点钟为 0°，顺时针
        const qreal span = (90 - gap) * grow;
        painter.setPen(QPen(arcColors[i], 0.036 * m_unit * (1 + 0.28 * pulse), Qt::SolidLine, Qt::FlatCap));
        // drawArc 以 3 点钟为 0°、逆时针为正，单位是 1/16°
        painter.drawArc(ringRect, qRound((90 - start) * 16), qRound(-span * 16));
    }

    // 12 点钟方向的三角标记
    const qreal mark = progress(t, 2150, 2500);
    if (mark > 0 && leave < 1) {
        const qreal tipY = m_cy - radius - 0.074 * m_unit - (1 - ease(OUT_BACK, mark)) * 0.04 * m_unit;
        const qreal hw = 0.02 * m_unit, hh = 0.03 * m_unit;
        painter.save();
        painter.setOpacity(painter.opacity() * qMin<qreal>(1, mark * 3) * (1 - leave));
        painter.setPen(Qt::NoPen);
        painter.setBrush(RED);
        painter.drawPolygon(QPolygonF({ QPointF(m_cx - hw, tipY - hh), QPointF(m_cx + hw, tipY - hh), QPointF(m_cx, tipY) }));
        painter.restore();
    }
}

// 每次闪烁时从圆环向外扩散的冲击波
void EventPointFullscreenReminder::drawShockwaves(QPainter& painter, qreal t, qreal radius) const
{
    const qreal maxRadius = std::hypot(m_width, m_height) / 2 + 20;
    const qreal duration = 1200;
    auto wave = [&](qreal age, int r, int g, int b, qreal alpha, qreal width) {
        if (age < 0 || age >= duration) return;
        const qreal p = age / duration;
        const qreal current = radius + (maxRadius - radius) * ease(OUT_CUBIC, p);
        painter.setPen(QPen(rgba(r, g, b, alpha * (1 - p)), width * (1 - p) + m_px));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(m_cx, m_cy), current, current);
    };
    for (int i = 0; i < m_flashTimes; i++) {
        const qreal pulse = PULSE_START + i * PULSE_GAP;
        wave(t - pulse, 255, 0, 0, 0.75, 0.012 * m_unit);
        wave(t - pulse - 140, 255, 255, 255, 0.3, 0.004 * m_unit);
    }
}

// 四角：倒计时全屏提醒的红色方块换成直角三角，位置、大小、节奏不变
void EventPointFullscreenReminder::drawCorners(QPainter& painter, qreal t) const
{
    const qreal appear = progress(t, 1600, 1950);
    if (appear <= 0) return;
    const qreal margin = 0.05 * m_unit, size = 0.1 * m_unit * ease(OUT_BACK, appear);
    const bool flash = isFlashing(t);
    const qreal bracketMargin = 0.022 * m_unit, bracketArm = 0.05 * m_unit;

    const struct Corner {
        qreal x, y, sx, sy;
    } corners[] = {
        { margin, margin, 1, 1 },
        { m_width - margin, margin, -1, 1 },
        { margin, m_height - margin, 1, -1 },
        { m_width - margin, m_height - margin, -1, -1 },
    };

    painter.save();
    painter.setOpacity(painter.opacity() * qMin<qreal>(1, appear * 2));
    for (const Corner& c : corners) {
        const QPolygonF triangle({ QPointF(c.x, c.y), QPointF(c.x + c.sx * size, c.y), QPointF(c.x, c.y + c.sy * size) });
        if (flash) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(RED);
        } else {
            painter.setPen(QPen(rgba(255, 0, 0, 0.5), qMax<qreal>(1, 1.5 * m_px)));
            painter.setBrush(Qt::NoBrush);
        }
        painter.drawPolygon(triangle);

        // 屏幕最外侧的 L 形角标
        const qreal bx = c.sx > 0 ? bracketMargin : m_width - bracketMargin;
        const qreal by = c.sy > 0 ? bracketMargin : m_height - bracketMargin;
        painter.setPen(QPen(rgba(255, 255, 255, 0.3), qMax<qreal>(1, 2 * m_px), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter.drawPolyline(QPolygonF({ QPointF(bx + c.sx * bracketArm, by), QPointF(bx, by), QPointF(bx, by + c.sy * bracketArm) }));
    }
    painter.restore();
}

// 上下两条斜纹警示带，从中间向两侧展开，条纹持续滚动
void EventPointFullscreenReminder::drawBands(QPainter& painter, qreal t) const
{
    const qreal reveal = ease(OUT_CUBIC, progress(t, 1250, 1850));
    if (reveal <= 0) return;
    const HudFrame frame = hudFrame(m_unit, m_width);
    if (frame.right - frame.left < 10) return;

    const qreal left = m_cx - (m_cx - frame.left) * reveal, right = m_cx + (frame.right - m_cx) * reveal;
    const qreal offset = t * 0.032 * m_unit / 900;
    drawStripes(painter, left, right, frame.margin, frame.band, offset, m_unit);
    drawStripes(painter, left, right, m_height - frame.margin - frame.band, frame.band, -offset, m_unit);

    const qreal line = qMax<qreal>(1, m_px);
    const QColor lineColor = rgba(255, 0, 0, 0.35);
    painter.fillRect(QRectF(left, frame.margin + frame.band + 0.008 * m_unit, right - left, line), lineColor);
    painter.fillRect(QRectF(left, m_height - frame.margin - frame.band - 0.008 * m_unit - line, right - left, line), lineColor);
}

// 圆环内文字：事件时间 / 事件名称 / 英文小字
void EventPointFullscreenReminder::drawTexts(QPainter& painter, qreal t, const Layout& lay) const
{
    auto leave = [&](qreal delay) { return progress(t, m_exit + delay, m_exit + delay + 360); };

    // 事件时间（红字，白块擦除）
    const QString time = m_eventPoint.time().toString("HH:mm:ss");
    const qreal timeSize = 0.066 * m_unit;
    const QFont timeFont = dinFont(timeSize, QFont::DemiBold, timeSize * 0.08);
    const qreal timeWidth = textWidth(timeFont, time);
    QPointF timeBase;
    const QRectF timeBox = placeText(timeFont, time, m_cx - timeWidth / 2, m_cy - 0.14 * m_unit, &timeBase);
    wipe(painter, timeBox.adjusted(-0.006 * m_unit, -0.008 * m_unit, 0.006 * m_unit, 0.008 * m_unit),
         progress(t, 1300, 1750), leave(120), Qt::white, [&] {
             painter.setFont(timeFont);
             painter.setPen(RED);
             painter.drawText(timeBase, time);
         });

    // 事件名称（白字，红块擦除）
    const QFont nameFont = cnFont(lay.nameSize, QFont::Black);
    const qreal nameWidth = textWidth(nameFont, lay.name);
    QPointF nameBase;
    const QRectF nameBox = placeText(nameFont, lay.name, m_cx - nameWidth / 2, m_cy + 0.012 * m_unit, &nameBase);
    wipe(painter, nameBox.adjusted(-0.01 * m_unit, -0.01 * m_unit, 0.01 * m_unit, 0.01 * m_unit),
         progress(t, 1450, 2050), leave(60), RED, [&] {
             painter.setFont(nameFont);
             painter.setPen(Qt::white);
             painter.drawText(nameBase, lay.name);
         });

    // 英文小字 + 红色竖条（对应倒计时页的 ColorLabel 和 "THE EXAM IN N DAYS"）
    const QString sub = QStringLiteral("EVENT POINT REACHED");
    const qreal subSize = qMax<qreal>(8, 0.025 * m_unit);
    const QFont subFont = dinFont(subSize, QFont::Medium, subSize * 0.32);
    const qreal barWidth = 0.006 * m_unit, barGap = 0.014 * m_unit;
    const qreal subWidth = barWidth + barGap + textWidth(subFont, sub);
    const qreal subLeft = m_cx - subWidth / 2, subY = m_cy + 0.155 * m_unit;
    const QRectF subBox(subLeft, subY - subSize * 0.75, subWidth, subSize * 1.5);
    const qreal subBaseline = subY + QFontMetricsF(subFont).capHeight() / 2;
    wipe(painter, subBox, progress(t, 1700, 2100), leave(0), RED, [&] {
        painter.fillRect(QRectF(subLeft, subBox.top(), barWidth, subBox.height()), RED);
        painter.setFont(subFont);
        painter.setPen(rgba(255, 255, 255, 0.78));
        painter.drawText(QPointF(subLeft + barWidth + barGap, subBaseline), sub);
    });
}

// 四周小字：标题、当前时间、下一个事件点、自动关闭进度
void EventPointFullscreenReminder::drawHud(QPainter& painter, qreal t) const
{
    const qreal appear = progress(t, 1800, 2200);
    if (appear <= 0) return;
    const HudFrame frame = hudFrame(m_unit, m_width);
    const qreal size = qMax<qreal>(9, 0.02 * m_unit);
    const QFont font = dinFont(size, QFont::DemiBold, size * 0.18);
    const qreal capHeight = QFontMetricsF(font).capHeight();
    const qreal topBaseline = frame.margin + frame.band + 0.032 * m_unit + capHeight / 2;
    const qreal bottomCenter = m_height - frame.margin - frame.band - 0.032 * m_unit;
    const qreal bottomBaseline = bottomCenter + capHeight / 2;
    const QColor ink = rgba(255, 255, 255, 0.55);

    painter.save();
    painter.setOpacity(painter.opacity() * appear);
    painter.setFont(font);
    painter.setPen(ink);

    painter.drawText(QPointF(frame.left, topBaseline), QStringLiteral("UNIVERSAL-TIMER // EVENT POINT"));
    const QString now = QStringLiteral("LOCAL ") + m_eventPoint.time().addSecs(int(std::floor(t / 1000))).toString("HH:mm:ss");
    painter.drawText(QPointF(frame.right - textWidth(font, now), topBaseline), now);

    // 下一个事件点（今天之后没有了就取明天第一个）
    const EventPoint* next = nullptr;
    bool tomorrow = false;
    for (const EventPoint& item : m_eventPointList) {
        if (item.time() > m_eventPoint.time() && (!next || item.time() < next->time())) next = &item;
    }
    if (!next) {
        tomorrow = true;
        for (const EventPoint& item : m_eventPointList) {
            if (item.time() != m_eventPoint.time() && (!next || item.time() < next->time())) next = &item;
        }
    }
    const QString head = QStringLiteral("NEXT ") + (next ? next->time().toString("HH:mm:ss") : QStringLiteral("--:--:--"));
    painter.drawText(QPointF(frame.left, bottomBaseline), head);
    if (next) {
        const QFont nameFont = cnFont(size, QFont::Medium);
        painter.setFont(nameFont);
        painter.drawText(QPointF(frame.left + textWidth(font, head) + size * 0.8, bottomBaseline),
                         next->name() + (tomorrow ? tr("（明日）") : QString()));
        painter.setFont(font);
    }

    // 自动关闭进度：分段条
    const int segments = 10;
    const qreal segmentWidth = 0.014 * m_unit, segmentHeight = 0.008 * m_unit, segmentGap = 0.004 * m_unit;
    const qreal barLeft = frame.right - (segments * segmentWidth + (segments - 1) * segmentGap);
    const int filled = int(std::ceil((1 - progress(t, 2100, m_exit)) * segments - 1e-6));
    for (int i = 0; i < segments; i++) {
        painter.fillRect(QRectF(barLeft + i * (segmentWidth + segmentGap), bottomCenter - segmentHeight / 2, segmentWidth, segmentHeight),
                         i < filled ? RED : rgba(255, 255, 255, 0.18));
    }
    const QString label = QStringLiteral("AUTO CLOSE");
    painter.drawText(QPointF(barLeft - 0.016 * m_unit - textWidth(font, label), bottomBaseline), label);
    painter.restore();
}
