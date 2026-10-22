#pragma once

#include "core/ConfigManager.h"
#include "ui/FloatingBar.h"

#include <QWidget>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

class FullscreenPagesManager : public QWidget // 终于把这玩意分离出来的我：我滴……任务……完！成！辣！啊哈哈哈哈哈哈哈！啊哈哈哈哈哈哈哈！
{
    Q_OBJECT

public:
    FullscreenPagesManager(QWidget* parent, ConfigManager& cfg, FloatingBarClass* bar);
    ~FullscreenPagesManager();

    void showReminder();
    void showSettings();
    void showWelcome(); // 显示欢迎页和引导

Q_SIGNALS:
    void onboardingFinished(); // 引导走完并淡出了

protected:
    void resizeEvent(QResizeEvent* event) override;

private:

    void startAnimation();

    // 强枚举
    // 全屏窗口模式
    enum class FullscreenPagesMode {
        None,
        Welcome,
        Settings,
        Reminder
    };
    FullscreenPagesMode fullscreen_pages_mode = FullscreenPagesMode::None;

    // Config
    ConfigManager& config;

    // FloatingBar
    FloatingBarClass* FloatingBar;

    // Widget
    QWidget* BackgroundWidget;
    QWidget* SlideAnimationWidget;

    // Animation
    QPropertyAnimation* SlideInAnimation;
    QPropertyAnimation* SlideOutAnimation;
    QPropertyAnimation* FadeOutAnimation;

    // AnimationGroup
    QSequentialAnimationGroup* SlideInOutAnimationGroup;
};