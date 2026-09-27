#pragma once

#include "../core/EventPoint.h"

#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QApplication>
#include <QScreen>

class EventPointReminder : public QLabel
{
    Q_OBJECT

public:
    explicit EventPointReminder(QWidget* parent, const EventPoint& eventPoint);


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
    QPropertyAnimation* m_adjustLabelAnimation;

    QSequentialAnimationGroup* m_fadeInGroup;
    QSequentialAnimationGroup* m_fadeOutGroup;


    void setupUI();
    void updateLabel();
    void setupAnimation();
    void showReminder();
};