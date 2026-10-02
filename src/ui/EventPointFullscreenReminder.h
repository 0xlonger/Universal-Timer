/*
    *  EventPointFullscreenReminder.h
    *  本文件由 Claude Opus 5.5 Extra 生成
*/
#pragma once

#include "../core/EventPoint.h"

#include <QWidget>
#include <QList>
#include <QRect>

class QPainter;
class QSoundEffect;
class QVariantAnimation;

// 事件点到达后的全屏提醒（整窗用 QPainter 绘制，由一条时间轴驱动）
//
// 时间轴（毫秒，0 = 到达事件点）：
//   0    ~ 300   顶部胶囊收窄成 5px 红线（承接 EventPointReminder）
//   300  ~ 650   红线贯穿全屏，背景压暗至 rgba(0, 0, 0, 0.75)
//   650  ~ 1020  从红线向两侧展开成满屏红色，接缝为 V 形
//   1020 ~ 2020  红门向左右分开，圆环 / 刻度 / 文字依次出现
//   2200 起      每 1000ms 一次：四角三角闪烁 500ms + 冲击波 + countdown.wav
//   exit ~ end   文字擦除、圆环收回、整体淡出 1000ms（InCubic）
class EventPointFullscreenReminder : public QWidget
{
    Q_OBJECT

public:
    // capsuleGeometry：EventPointReminder 胶囊在屏幕上的位置，作为红线的起点；为空时用默认位置
    // flashTimes：闪烁 / 提示音次数，0 表示不闪烁，停留 2.5 秒
    explicit EventPointFullscreenReminder(const EventPoint& eventPoint,
                                          const QList<EventPoint>& eventPointList = {},
                                          const QRect& capsuleGeometry = QRect(),
                                          int flashTimes = 3,
                                          QWidget* parent = nullptr);

    void start();                 // 显示并从 0 开始播放
    void seek(qreal ms);          // 停在某一时刻（预览、截图用）
    qreal totalDuration() const;  // 从 0 到完全淡出的时长

Q_SIGNALS:
    void finished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override; // 点击后直接进入收尾

private:
    struct Layout {
        qreal radius;      // 圆环半径
        qreal nameSize;    // 事件名称字号
        QString name;
    };

    void updateMetrics();
    Layout layout() const;
    QRectF capsuleRect() const;
    qreal pulseEnvelope(qreal t) const;
    bool isFlashing(qreal t) const;

    void drawCapsule(QPainter& painter, qreal t) const;
    void drawStrike(QPainter& painter, qreal t) const;
    void drawDoors(QPainter& painter, qreal t) const;
    void drawRing(QPainter& painter, qreal t, qreal radius) const;
    void drawShockwaves(QPainter& painter, qreal t, qreal radius) const;
    void drawCorners(QPainter& painter, qreal t) const;
    void drawBands(QPainter& painter, qreal t) const;
    void drawTexts(QPainter& painter, qreal t, const Layout& lay) const;
    void drawHud(QPainter& painter, qreal t) const;

    EventPoint m_eventPoint;
    QList<EventPoint> m_eventPointList;
    QRect m_capsuleGeometry;  // 屏幕坐标
    int m_flashTimes;

    qreal m_t = 0;            // 当前时刻（毫秒）
    qreal m_exit = 0;         // 开始收尾的时刻
    qreal m_end = 0;          // 完全淡出的时刻

    // 每帧根据窗口大小计算
    qreal m_width = 1, m_height = 1;
    qreal m_unit = 1;         // 以 16:9 屏幕高度为基准的长度单位（1080p 下等于 1080px）
    qreal m_px = 1;           // m_unit / 1080，1080p 下 1px
    qreal m_cx = 0, m_cy = 0;

    QVariantAnimation* m_timeline;
    QSoundEffect* m_countdownSound;
};
