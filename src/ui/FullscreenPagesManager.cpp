#include "ui/FullscreenPagesManager.h"
#include "ui/ReminderPage.h"
#include "ui/SettingsPage.h"
#include "ui/WelcomePage.h"
#include "core/ThemeManager.h"

#include <QTimer>

FullscreenPagesManager::FullscreenPagesManager(QWidget* parent, ConfigManager& cfg, FloatingBarClass* bar)
    : QWidget(parent), config(cfg), FloatingBar(bar)
{

    this->setAttribute(Qt::WA_TranslucentBackground);

    BackgroundWidget = new QWidget(this);
    SlideAnimationWidget = new QWidget(this);
    auto applyTheme = [this] {
        BackgroundWidget->setStyleSheet(ThemeManager::instance().style("FullscreenBackground"));
        SlideAnimationWidget->setStyleSheet(ThemeManager::instance().style("FullscreenSlide"));
        };
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::changed, this, applyTheme);
    BackgroundWidget->hide();

    SlideInAnimation = new QPropertyAnimation(SlideAnimationWidget, "pos", this);
    SlideInAnimation->setDuration(250);
    SlideOutAnimation = new QPropertyAnimation(SlideAnimationWidget, "pos", this);
    SlideOutAnimation->setDuration(1000);
    SlideOutAnimation->setEasingCurve(QEasingCurve::OutCubic);
    SlideInOutAnimationGroup = new QSequentialAnimationGroup(this);
    SlideInOutAnimationGroup->addAnimation(SlideInAnimation);
    SlideInOutAnimationGroup->addAnimation(SlideOutAnimation);

    FadeOutAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    FadeOutAnimation->setDuration(1000);
    FadeOutAnimation->setStartValue(1);
    FadeOutAnimation->setEndValue(0);
    FadeOutAnimation->setEasingCurve(QEasingCurve::InCubic);

    connect(SlideInAnimation, &QPropertyAnimation::finished, [this] {
        switch (fullscreen_pages_mode) {
            case FullscreenPagesMode::Reminder: {
                ReminderPageClass* ReminderPage = new ReminderPageClass(this, config);
                ReminderPage->move((this->width() - ReminderPage->width()) / 2, (this->height() - ReminderPage->height()) / 2);
                SlideAnimationWidget->raise();
                BackgroundWidget->show();
                ReminderPage->show();
                connect(ReminderPage, &ReminderPageClass::finished, this, [this, ReminderPage] {
                    FadeOutAnimation->start();
                    connect(FadeOutAnimation, &QPropertyAnimation::finished, ReminderPage, &ReminderPageClass::deleteLater, Qt::SingleShotConnection);
                    }, Qt::SingleShotConnection);
                break;
            }
            case FullscreenPagesMode::Settings: {
                SettingsPageClass* SettingsPage = new SettingsPageClass(this, config, FloatingBar);
                SettingsPage->resize(this->size());
                SlideAnimationWidget->raise();
                BackgroundWidget->show();
                SettingsPage->show();
                connect(SettingsPage, &SettingsPageClass::clickedCloseButton, this, [this, SettingsPage] {
                    FadeOutAnimation->start();
                    connect(FadeOutAnimation, &QPropertyAnimation::finished, SettingsPage, &SettingsPageClass::deleteLater, Qt::SingleShotConnection);
                    }, Qt::SingleShotConnection);
                connect(SettingsPage, &SettingsPageClass::clickedReminderPreviewButton, this, [this, SettingsPage] {
                    fullscreen_pages_mode = FullscreenPagesMode::None;
                    showReminder();
                    connect(SlideInAnimation, &QPropertyAnimation::finished, SettingsPage, &SettingsPageClass::deleteLater, Qt::SingleShotConnection);
                    }, Qt::SingleShotConnection);
                break;
            }
            case FullscreenPagesMode::Welcome: {
                WelcomePageClass* WelcomePage = new WelcomePageClass(this);
                WelcomePage->resize(this->size());
                SlideAnimationWidget->raise();
                BackgroundWidget->show();
                WelcomePage->show();
                connect(WelcomePage, &WelcomePageClass::finished, this, [this, WelcomePage] {
                    FadeOutAnimation->start();
                    connect(FadeOutAnimation, &QPropertyAnimation::finished, WelcomePage, &WelcomePageClass::deleteLater, Qt::SingleShotConnection);
                    }, Qt::SingleShotConnection);
                break;
            }
        }
        });
    connect(FadeOutAnimation, &QPropertyAnimation::finished, [this] {
        this->hide();
        BackgroundWidget->hide();
        fullscreen_pages_mode = FullscreenPagesMode::None;
        });
}

FullscreenPagesManager::~FullscreenPagesManager()
{}

void FullscreenPagesManager::showReminder() {
    if (fullscreen_pages_mode == FullscreenPagesMode::None) {
        fullscreen_pages_mode = FullscreenPagesMode::Reminder;
        startAnimation();
    }
}

void FullscreenPagesManager::showSettings() {
    if (fullscreen_pages_mode == FullscreenPagesMode::None) {
        fullscreen_pages_mode = FullscreenPagesMode::Settings;
        startAnimation();
    }
}

void FullscreenPagesManager::showWelcome() {
    if (fullscreen_pages_mode == FullscreenPagesMode::None) {
        fullscreen_pages_mode = FullscreenPagesMode::Welcome;
        startAnimation();
    }
}

void FullscreenPagesManager::startAnimation() {
    this->setWindowOpacity(1);
    this->show();
    SlideInOutAnimationGroup->start();
}

void FullscreenPagesManager::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    BackgroundWidget->resize(this->size());

    SlideAnimationWidget->resize(event->size());

    SlideInAnimation->setStartValue(QPoint(-event->size().width(), 0));
    SlideInAnimation->setEndValue(QPoint(0, 0));
    SlideOutAnimation->setStartValue(QPoint(0, 0));
    SlideOutAnimation->setEndValue(QPoint(event->size().width(), 0));
    
}