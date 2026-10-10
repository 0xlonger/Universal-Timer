#pragma once

#include "core/ConfigManager.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

// 定时全屏提醒的时间列表（reminder_time_list.txt）编辑器：每个时间一行，可以添加、删除，改了立即保存
class ReminderTimeListEditorClass : public QWidget
{
    Q_OBJECT

public:
    ReminderTimeListEditorClass(QWidget* parent, ConfigManager& cfg);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    ConfigManager& config;
    QScrollArea* ScrollArea;
    QWidget* RowsWidget; // 放所有行的容器（在滚动区域里）
    QVBoxLayout* RowsLayout;

    void rebuild(); // 按时间列表重新生成所有行
    void scheduleRebuild(); // 增删后调用（不能在按钮自己的信号里直接删除它）
};
