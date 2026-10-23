#pragma once

#include "core/ConfigManager.h"
#include "ui/FloatingBar.h"
#include "ui/FullscreenPagesManager.h"
#include "ui/AboutPage.h"

#include <QSystemTrayIcon>
#include <QTimer>

class UniversalTimer2 : public QObject
{
    Q_OBJECT

public:
    UniversalTimer2(QObject* parent = nullptr);
    ~UniversalTimer2();

private:


    // 函数声明
    // 刷新
    void refresh();

    // 更新
    void updateFloatingBar(); // 更新悬浮条
    void updateObjects(); // 更新对象
    void scheduleNextUpdate(); // 安排下一次更新：下一个整 update_interval 刚过 50ms
    void checkReminders(); // 到时间就显示定时全屏提醒和事件点提醒
    void scheduleNextReminderCheck(); // 安排下一次检查提醒：加上时间偏移后的下一个整 update_interval 刚过 50ms
    QTime offsetTime() const; // 加上时间偏移后的当前时间
    void applyTimeAutoAdjust(); // 自动时间偏移：过了几天就把时间偏移加上几天的增量


    // 屏幕分辨率
    QRect desktop;

    // 计时器
    QTimer timer;
    QTimer reminder_timer; // 提醒按加上时间偏移后的整秒检查，偏移有小数时和悬浮条的计时器错开

    // 配置
    ConfigManager config;

    // 悬浮条
    FloatingBarClass* FloatingBar;

    // 全屏窗口
    FullscreenPagesManager* FullscreenPages;

    // About Page
    AboutPageClass* AboutPage;

    // SystemTrayIcon
    QSystemTrayIcon* TrayIcon; // 系统托盘图标


};
