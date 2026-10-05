#pragma once

#include "EventPoint.h"

#include <QDateTime>
#include <QSettings>

enum class FloatingBarPosition {
    TopLeft,
    TopCenter,
    TopRight
};


class ConfigManager
{

private:

    struct GeneralConfigItems {
        QDateTime target_date_time = QDateTime(QDate(2025, 6, 30), QTime(0, 0, 0)); // 目标时间，默认2025-06-30 00:00:00
        unsigned update_interval = 1000; // 更新间隔时间，默认1秒
        QString language = "zh-CN"; // 语言，默认中文
    };

    struct FloatingBarConfigItems {
        bool is_show_floating_bar = true; // 是否显示悬浮条，默认显示
        bool floating_bar_on_top = true; // 悬浮条是否置顶，默认置顶
        FloatingBarPosition floating_bar_position = FloatingBarPosition::TopCenter; // 悬浮条位置，默认居中
        unsigned floating_bar_border_radius = 10; // 悬浮条圆角，默认10
        unsigned floating_bar_height = 50; // 悬浮条高度，默认50
        QString floating_bar_text = "距会考还剩："; // 悬浮条文本
    };

    struct ReminderConfigItems {
        bool is_show_reminder = true; // 是否显示全屏提醒
        QString reminder_text = "距会考"; // 全屏提醒文本
        QString reminder_small_text = "THE EXAM IN "; // 全屏提醒小文本
        int remaining_days_to_play_countdown_sound = 30; // 剩余天数播放倒计时声音，默认30
        int remaining_days_to_play_heartbeat_sound = 14; // 剩余天数播放心跳声音，默认14
        unsigned block_show_times = 4; // 红色块显示次数，默认4
        QList<QTime> reminder_time_list = { QTime(8, 13), QTime(9, 3), QTime(9, 53), QTime(10, 43), QTime(11, 38), QTime(14, 20), QTime(15, 3), QTime(15, 53), QTime(16, 43), QTime(18, 3) }; // 时间列表
    };

    struct EventPointConfigItems {
        QList<EventPoint> event_point_list = { EventPoint(0, QTime(11, 55), "放学", 60), EventPoint(1, QTime(17, 40), "放学", 60), EventPoint(2, QTime(22, 0), "放学", 60) }; // 事件点列表
    };


    QSettings Settings = QSettings("config.ini", QSettings::IniFormat);


public:
    ConfigManager();
    ~ConfigManager();

    GeneralConfigItems general;

    FloatingBarConfigItems floating_bar;

    ReminderConfigItems reminder;

    EventPointConfigItems event_point;

    template<typename T1, typename T2> void set(T1& item, const T2& variant) {
        item = variant;
        write();
    }

    void read();
    void write();

};