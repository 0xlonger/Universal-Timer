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
    font.setFamilies({ "DIN1451", "Bahnschrift", "Barlow Condensed", "Arial Narrow" });
    font.setPixelSize(desktop.width() * 0.05 * GOLDEN_RATIO_INV);
    m_label->setFont(font);

    m_opacityEffect = new QGraphicsOpacityEffect(m_label);
    m_opacityEffect->setOpacity(1.0);
    m_label->setGraphicsEffect(m_opacityEffect);
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

    m_countdownAnimation1 = new QVariantAnimation(m_label);
    m_countdownAnimation1->setDuration(250);
    m_countdownAnimation1->setEasingCurve(QEasingCurve::OutCubic);
    m_countdownAnimation1->setStartValue(desktop.width() * 0.05);
    m_countdownAnimation1->setEndValue(desktop.width() * 0.05 * GOLDEN_RATIO_INV);
    connect(m_countdownAnimation1, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        QFont font = m_label->font();
        font.setPixelSize(value.toInt());
        m_label->setFont(font);
    });

    m_countdownAnimation2 = new QVariantAnimation(m_label);
    m_countdownAnimation2->setDuration(250);
    m_countdownAnimation2->setEasingCurve(QEasingCurve::InCubic);
    m_countdownAnimation2->setStartValue(desktop.width() * 0.05 * GOLDEN_RATIO_INV);
    m_countdownAnimation2->setEndValue(desktop.width() * 0.05 * GOLDEN_RATIO_INV * GOLDEN_RATIO_INV);
    connect(m_countdownAnimation2, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        QFont font = m_label->font();
        font.setPixelSize(value.toInt());
        m_label->setFont(font);
    });

    m_countdownOpacityAnimation1 = new QPropertyAnimation(m_opacityEffect, "opacity");
    m_countdownOpacityAnimation1->setDuration(250);
    m_countdownOpacityAnimation1->setEasingCurve(QEasingCurve::OutCubic);
    m_countdownOpacityAnimation1->setStartValue(0.0);
    m_countdownOpacityAnimation1->setEndValue(1.0);

    m_countdownOpacityAnimation2 = new QPropertyAnimation(m_opacityEffect, "opacity");
    m_countdownOpacityAnimation2->setDuration(250);
    m_countdownOpacityAnimation2->setEasingCurve(QEasingCurve::InCubic);
    m_countdownOpacityAnimation2->setStartValue(1.0);
    m_countdownOpacityAnimation2->setEndValue(0.0);

    m_fadeInGroup = new QSequentialAnimationGroup(this);
    m_fadeInGroup->addAnimation(m_fadeInAnimation1);
    m_fadeInGroup->addAnimation(m_fadeInAnimation2);

    m_fadeOutGroup = new QSequentialAnimationGroup(this);
    m_fadeOutGroup->addAnimation(m_fadeOutAnimation1);
    m_fadeOutGroup->addAnimation(m_fadeOutAnimation2);

    m_countdownGroup1 = new QParallelAnimationGroup(this);
    m_countdownGroup1->addAnimation(m_countdownAnimation1);
    m_countdownGroup1->addAnimation(m_countdownOpacityAnimation1);

    m_countdownGroup2 = new QParallelAnimationGroup(this);
    m_countdownGroup2->addAnimation(m_countdownAnimation2);
    m_countdownGroup2->addAnimation(m_countdownOpacityAnimation2);

    m_countdownGroup = new QSequentialAnimationGroup(this);
    m_countdownGroup->addAnimation(m_countdownGroup1);
    m_countdownGroup->addPause(500);
    m_countdownGroup->addAnimation(m_countdownGroup2);
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
    // 只量出文字需要的宽度，不改 m_label 的大小（它始终铺满胶囊，见 resizeEvent）
    const int labelWidth = m_label->sizeHint().width();
    m_adjustAnimation->setStartValue(this->geometry());
    m_adjustAnimation->setEndValue(QRect((desktop.width() - ((labelWidth + 30) > (desktop.width() * 0.05) ? (labelWidth + 30) : (desktop.width() * 0.05))) / 2, desktop.height() * 0.1, (labelWidth + 30) > (desktop.width() * 0.05) ? (labelWidth + 30) : (desktop.width() * 0.05), this->height()));
    m_adjustAnimation->start();
    m_countdownGroup->start();
}

void EventPointReminder::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);
    // 胶囊本身设置了 WA_TranslucentBackground，Qt 不会绘制它自己的样式表背景和红边，
    // 屏幕上看到的黑底红边其实是 m_label。让 m_label 始终铺满胶囊，展开和宽度变化的动画才能完整显示
    m_label->setGeometry(this->rect());
}