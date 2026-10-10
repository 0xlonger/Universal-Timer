#include "ui/ReminderTimeListEditor.h"

#include <QScrollArea>
#include <QHBoxLayout>
#include <QTimeEdit>
#include <QPushButton>
#include <QTimer>
#include <QScrollBar>
#include <QWheelEvent>

ReminderTimeListEditorClass::ReminderTimeListEditorClass(QWidget* parent, ConfigManager& cfg)
    : QWidget(parent), config(cfg)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    ScrollArea = new QScrollArea(this);
    ScrollArea->setWidgetResizable(true);
    ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    RowsWidget = new QWidget(ScrollArea);
    RowsLayout = new QVBoxLayout(RowsWidget);
    RowsLayout->setContentsMargins(0, 0, 0, 0);
    ScrollArea->setWidget(RowsWidget);
    layout->addWidget(ScrollArea, 1);

    QPushButton* addButton = new QPushButton(tr("添加提醒时间"), this);
    connect(addButton, &QPushButton::clicked, this, [this] {
        // 新的提醒时间默认是 10 分钟后的整分钟
        const QTime time = QTime::currentTime().addSecs(600);
        config.reminder.reminder_time_list.append(QTime(time.hour(), time.minute()));
        config.write();
        scheduleRebuild();
        });
    layout->addWidget(addButton);

    rebuild();
}

void ReminderTimeListEditorClass::scheduleRebuild()
{
    QTimer::singleShot(0, this, &ReminderTimeListEditorClass::rebuild);
}

bool ReminderTimeListEditorClass::eventFilter(QObject* watched, QEvent* event)
{
    // 没点进去的输入框不响应滚轮，滚轮用来滚动列表，免得滚动时误改了时间和数字
    if (event->type() == QEvent::Wheel && !static_cast<QWidget*>(watched)->hasFocus()) {
        QCoreApplication::sendEvent(ScrollArea->verticalScrollBar(), event);
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void ReminderTimeListEditorClass::rebuild()
{
    while (QLayoutItem* item = RowsLayout->takeAt(0)) {
        if (QWidget* widget = item->widget())
            widget->deleteLater();
        delete item;
    }

    for (int i = 0; i < config.reminder.reminder_time_list.size(); i++) {
        QWidget* row = new QWidget(RowsWidget);
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        QTimeEdit* timeEdit = new QTimeEdit(config.reminder.reminder_time_list[i], row);
        timeEdit->setDisplayFormat("HH:mm:ss");
        connect(timeEdit, &QTimeEdit::timeChanged, this, [this, i](const QTime& time) {
            if (i < config.reminder.reminder_time_list.size()) {
                config.reminder.reminder_time_list[i] = time;
                config.write();
            }
            });
        QPushButton* deleteButton = new QPushButton(tr("删除"), row);
        connect(deleteButton, &QPushButton::clicked, this, [this, i] {
            if (i < config.reminder.reminder_time_list.size()) {
                config.reminder.reminder_time_list.removeAt(i);
                config.write();
            }
            scheduleRebuild();
            });

        rowLayout->addWidget(timeEdit, 1);
        rowLayout->addWidget(deleteButton);
        // 设置中心有样式表时字体不会传给后来新建的控件，所以手动设成和其他控件一样的字体（见 SettingsContentClass::resizeEvent）
        row->setFont(RowsWidget->font());
        for (QWidget* child : row->findChildren<QWidget*>())
            child->setFont(RowsWidget->font());
        for (QAbstractSpinBox* spinBox : row->findChildren<QAbstractSpinBox*>()) {
            spinBox->setFocusPolicy(Qt::StrongFocus);
            spinBox->installEventFilter(this);
        }
        RowsLayout->addWidget(row);
    }
    RowsLayout->addStretch();
}
