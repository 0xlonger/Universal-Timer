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


    // 屏幕分辨率
    QRect desktop;

    // 计时器
    QTimer timer;

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
