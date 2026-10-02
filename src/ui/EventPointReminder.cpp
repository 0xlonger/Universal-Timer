#include "EventPointReminder.h"
#include "../core/Global.h"

EventPointReminder::EventPointReminder(QWidget* parent, const EventPoint& eventPoint)
    : QLabel(parent), m_eventPoint(eventPoint)
{
    setupUI();
    setupAnimation();
    showReminder();
}

void EventPointReminder::setupUI()
{
    this->setStyleSheet("background-color: rgba(0, 0, 0, 0.75); color: white; border-left: 5px solid red; border-right: 5px solid red;");
    this->setAlignment(Qt::AlignCenter);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setPixelSize(desktop.width() * 0.05 * GOLDEN_RATIO_INV);
    m_label->setFont(font);
}

void EventPointReminder::setupAnimation()
{
    m_fadeInAnimation1 = new QPropertyAnimation(this, "geometry");
    m_fadeInAnimation1->setDuration(500);
    m_fadeInAnimation1->setEasingCurve(QEasingCurve::OutCubic);
    m_fadeInAnimation1->setStartValue(QRect((desktop.width() - 5) / 2, desktop.height() * 0.1 + desktop.width() * 0.05 / 2, 5, 0));
    m_fadeInAnimation1->setEndValue(QRect((desktop.width() - 5) / 2, desktop.height() * 0.1, 5, desktop.width() * 0.05));
    
    m_fadeInAnimation2 = new QPropertyAnimation(this, "geometry");
    m_fadeInAnimation2->setDuration(500);
    m_fadeInAnimation2->setEasingCurve(QEasingCurve::OutCubic);
    m_fadeInAnimation2->setStartValue(QRect((desktop.width() - 5) / 2, desktop.height() * 0.1, 5, desktop.width() * 0.05));
    m_fadeInAnimation2->setEndValue(QRect((desktop.width() - desktop.width() * 0.05) / 2, desktop.height() * 0.1, desktop.width() * 0.05, desktop.width() * 0.05));

    m_fadeOutAnimation1 = new QPropertyAnimation(this, "geometry");
    m_fadeOutAnimation1->setDuration(500);
    m_fadeOutAnimation1->setEasingCurve(QEasingCurve::OutCubic);
    m_fadeOutAnimation1->setStartValue(QRect((desktop.width() - desktop.width() * 0.05) / 2, desktop.height() * 0.1, desktop.width() * 0.05, desktop.width() * 0.05));
    m_fadeOutAnimation1->setEndValue(QRect((desktop.width() - 5) / 2, desktop.height() * 0.1, 5, desktop.width() * 0.05));

    m_fadeOutAnimation2 = new QPropertyAnimation(this, "geometry");
    m_fadeOutAnimation2->setDuration(500);
    m_fadeOutAnimation2->setEasingCurve(QEasingCurve::OutCubic);
    m_fadeOutAnimation2->setStartValue(QRect((desktop.width() - 5) / 2, desktop.height() * 0.1, 5, desktop.width() * 0.05));
    m_fadeOutAnimation2->setEndValue(QRect((desktop.width() - 5) / 2, desktop.height() * 0.1 + desktop.width() * 0.05 / 2, 5, 0));

    m_adjustAnimation = new QPropertyAnimation(this, "geometry");
    m_adjustAnimation->setDuration(500);
    m_adjustAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_adjustLabelAnimation = new QPropertyAnimation(m_label, "size");
    m_adjustLabelAnimation->setDuration(500);
    m_adjustLabelAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_fadeInGroup = new QSequentialAnimationGroup(this);
    m_fadeInGroup->addAnimation(m_fadeInAnimation1);
    m_fadeInGroup->addAnimation(m_fadeInAnimation2);

    m_fadeOutGroup = new QSequentialAnimationGroup(this);
    m_fadeOutGroup->addAnimation(m_fadeOutAnimation1);
    m_fadeOutGroup->addAnimation(m_fadeOutAnimation2);
}

void EventPointReminder::showReminder()
{
    this->show();
    m_fadeInGroup->start();
    m_timer = new QTimer(this);
    m_timer->start(1000);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        updateLabel();
    });
}

void EventPointReminder::updateLabel()
{
    QTime currentTime = QTime::currentTime();
    int remainingSeconds = currentTime.secsTo(m_eventPoint.time());
    if (remainingSeconds <= 0) {
        // 到达事件点：把胶囊当前位置交给全屏提醒，红线从这里开始
        m_timer->stop();
        emit reached(m_eventPoint, this->geometry());
        this->hide();
        this->deleteLater();
        return;
    }
    m_label->setText(QString::number(remainingSeconds));
    m_label->adjustSize();
    m_adjustAnimation->setStartValue(this->geometry());
    m_adjustAnimation->setEndValue(QRect((desktop.width() - m_label->width() / GOLDEN_RATIO_INV) / 2, desktop.height() * 0.1, m_label->width() / GOLDEN_RATIO_INV, this->height()));
    m_adjustLabelAnimation->setStartValue(this->size());
    m_adjustLabelAnimation->setEndValue(QSize(m_label->width() / GOLDEN_RATIO_INV, this->height()));
    m_adjustAnimation->start();
    m_adjustLabelAnimation->start();
}