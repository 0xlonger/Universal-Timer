#include "core/ConfigManager.h"
#include "core/LogManager.h"

#include <QObject>
#include <QMessageBox>

ConfigManager::ConfigManager()
{
    read();
}

ConfigManager::~ConfigManager()
{}


void ConfigManager::read() {
    if (QFile::exists("config.ini")) {
        qDebug() << "读取配置文件……";

        Settings.beginGroup("main");
        general.target_date_time = Settings.value("target_date_time", QDateTime(QDate(2025, 6, 30), QTime(0, 0, 0))).toDateTime();
        general.update_interval = Settings.value("update_interval", 1000).toInt();
        general.language = Settings.value("language", "zh-CN").toString();
        general.theme = Settings.value("theme", "default").toString();
        Settings.endGroup();

        Settings.beginGroup("floating_bar");
        floating_bar.is_show_floating_bar = Settings.value("is_show_floating_bar", true).toBool();
        floating_bar.floating_bar_on_top = Settings.value("floating_bar_on_top", true).toBool();
        floating_bar.floating_bar_position = static_cast<FloatingBarPosition>(Settings.value("floating_bar_position", static_cast<int>(FloatingBarPosition::TopCenter)).toInt());
        floating_bar.floating_bar_border_radius = Settings.value("floating_bar_border_radius", 10).toUInt();
        floating_bar.floating_bar_height = Settings.value("floating_bar_height", 50).toUInt();
        floating_bar.floating_bar_text = Settings.value("floating_bar_text", "距会考还剩：").toString();
        Settings.endGroup();

        Settings.beginGroup("reminder");
        reminder.is_show_reminder = Settings.value("is_show_reminder", true).toBool();
        reminder.reminder_text = Settings.value("reminder_text", "距会考").toString();
        reminder.reminder_small_text = Settings.value("reminder_small_text", "THE EXAM IN ").toString();
        reminder.remaining_days_to_play_countdown_sound = Settings.value("remaining_days_to_play_countdown_sound", 30).toInt();
        reminder.remaining_days_to_play_heartbeat_sound = Settings.value("remaining_days_to_play_heartbeat_sound", 14).toInt();
        reminder.block_show_times = Settings.value("block_show_times", 4).toUInt();
        //reminder.reminder_time_list = Settings.value("reminder_time_list").toList(); // 暂时使用另一方法读取时间列表 --- IGNORE ---
        Settings.endGroup();

        QFile time_list_file("reminder_time_list.txt");
        if (time_list_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            reminder.reminder_time_list.clear();
            while (!time_list_file.atEnd()) {
                QString line = time_list_file.readLine().trimmed();
                if (line.isEmpty()) continue;
                QTime time = QTime::fromString(line, "HH:mm:ss");
                reminder.reminder_time_list.append(time);
            }
            time_list_file.close();
        }

        QFile event_point_list_file("event_point_list.txt");
        // 格式：ID;时间;事件名称;提前时间（秒）;闪烁次数
        if (event_point_list_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            event_point.event_point_list.clear();
            while (!event_point_list_file.atEnd()) {
                QString line = event_point_list_file.readLine().trimmed();
                if (line.isEmpty()) continue;
                QStringList parts = line.split(';');
                if (parts.size() == 5) {
                    int id = parts[0].toInt();
                    QTime time = QTime::fromString(parts[1], "HH:mm:ss");
                    QString name = parts[2];
                    int advanceTime = parts[3].toInt();
                    unsigned flashTimes = parts[4].toUInt();
                    event_point.event_point_list.append(EventPoint(id, time, name, advanceTime, flashTimes));
                }
            }
            event_point_list_file.close();
        }

        qInfo() << "配置文件读取成功";
    }
}

void ConfigManager::write() {
    qDebug() << "写入配置文件……";

    Settings.beginGroup("main");
    Settings.setValue("target_date_time", general.target_date_time);
    Settings.setValue("update_interval", general.update_interval);
    Settings.setValue("language", general.language);
    Settings.setValue("theme", general.theme);
    Settings.endGroup();

    Settings.beginGroup("floating_bar");
    Settings.setValue("is_show_floating_bar", floating_bar.is_show_floating_bar);
    Settings.setValue("floating_bar_on_top", floating_bar.floating_bar_on_top);
    Settings.setValue("floating_bar_position", static_cast<int>(floating_bar.floating_bar_position));
    Settings.setValue("floating_bar_border_radius", floating_bar.floating_bar_border_radius);
    Settings.setValue("floating_bar_height", floating_bar.floating_bar_height);
    Settings.setValue("floating_bar_text", floating_bar.floating_bar_text);
    Settings.endGroup();

    Settings.beginGroup("reminder");
    Settings.setValue("is_show_reminder", reminder.is_show_reminder);
    Settings.setValue("reminder_text", reminder.reminder_text);
    Settings.setValue("reminder_small_text", reminder.reminder_small_text);
    Settings.setValue("remaining_days_to_play_countdown_sound", reminder.remaining_days_to_play_countdown_sound);
    Settings.setValue("remaining_days_to_play_heartbeat_sound", reminder.remaining_days_to_play_heartbeat_sound);
    Settings.setValue("block_show_times", reminder.block_show_times);
    //Settings.setValue("reminder_time_list", reminder.reminder_time_list);
    Settings.endGroup();

    Settings.sync();

    QFile time_list_file("reminder_time_list.txt");
    if (time_list_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&time_list_file);
        for (const QTime& time : reminder.reminder_time_list) {
            out << time.toString("HH:mm:ss") << Qt::endl;
        }
        time_list_file.close();
    }

    QFile event_point_list_file("event_point_list.txt");
    if (event_point_list_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&event_point_list_file);
        for (const EventPoint& event_point_item : event_point.event_point_list) {
            out  << event_point_item.id() << ";" << event_point_item.time().toString("HH:mm:ss") << ";" << event_point_item.name() << ";" << event_point_item.advanceTime() << ";" << event_point_item.flashTimes() << Qt::endl;
        }
        event_point_list_file.close();
    }

    if (Settings.status() != QSettings::NoError) {
        QMessageBox::critical(NULL, QObject::tr("错误"), QObject::tr("配置文件写入失败<br>错误代码：") + QString::number(Settings.status()), QMessageBox::Ok);
        qWarning() << "配置文件写入失败，错误代码：" << QString::number(Settings.status());
    }
    else {
        qInfo() << "配置文件写入成功";
    }
}