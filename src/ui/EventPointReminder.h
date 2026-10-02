#pragma once

#include "../core/EventPoint.h"

#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QApplication>
#include <QScreen>
#include <QResizeEvent>

class EventPointReminder : public QLabel
{
    Q_OBJECT

public:
    explicit EventPointReminder(QWidget* parent, const EventPoint& eventPoint);

protected:

    void resizeEvent(QResizeEvent* event) override;

Q_SIGNALS:

    void reached(const EventPoint& eventPoint, const QRect& capsuleGeometry); // 到达事件点，交给全屏提醒继续播放


private:

    QRect desktop = QApplication::primaryScreen()->geometry();

    QLabel* m_label;

    EventPoint m_eventPoint;
    QTimer* m_timer;
    QPropertyAnimation* m_animation;
    QGraphicsOpacityEffect* m_opacityEffect;

    QPropertyAnimation* m_fadeInAnimation1;
    QPropertyAnimation* m_fadeInAnimation2;
    QPropertyAnimation* m_fadeOutAnimation1;
    QPropertyAnimation* m_fadeOutAnimation2;
    QPropertyAnimation* m_adjustAnimation;

    QVariantAnimation* m_countdownAnimation1;
    QVariantAnimation* m_countdownAnimation2;
    QPropertyAnimation* m_countdownOpacityAnimation1;
    QPropertyAnimation* m_countdownOpacityAnimation2;

    QSequentialAnimationGroup* m_fadeInGroup;
    QSequentialAnimationGroup* m_fadeOutGroup;
    QParallelAnimationGroup* m_countdownGroup1;
    QParallelAnimationGroup* m_countdownGroup2;
    QSequentialAnimationGroup* m_countdownGroup;


    void setupUI();
    void updateLabel();
    void setupAnimation();
    void showReminder();
};