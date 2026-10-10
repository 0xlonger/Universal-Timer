#pragma once

#include "core/ConfigManager.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

// 事件点列表（event_point_list.txt）编辑器：每个事件点一行（时间、名称、提前秒数、闪烁次数），可以添加、删除、预览，改了立即保存
class EventPointListEditorClass : public QWidget
{
    Q_OBJECT

public:
    EventPointListEditorClass(QWidget* parent, ConfigManager& cfg);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    ConfigManager& config;
    QScrollArea* ScrollArea;
    QWidget* RowsWidget; // 放所有行的容器（在滚动区域里）
    QVBoxLayout* RowsLayout;

    void rebuild(); // 按事件点列表重新生成所有行
    void scheduleRebuild(); // 增删后调用（不能在按钮自己的信号里直接删除它）
    void updateEventPoint(int index, const QTime& time, const QString& name, int advanceTime, unsigned flashTimes);
    void preview(int index); // 立即用这个事件点播放一遍倒数胶囊和事件点全屏提醒
};
