#include "ui/EventPointListEditor.h"
#include "ui/EventPointReminder.h"
#include "ui/EventPointFullscreenReminder.h"

#include <QScrollArea>
#include <QHBoxLayout>
#include <QTimeEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <QScrollBar>
#include <QWheelEvent>

static constexpr int PREVIEW_SECONDS = 5; // 预览时倒数的秒数
static constexpr unsigned PREVIEW_EVENT_POINT_ID = 0x3f3f3f3e; // 预览用的事件点 ID，不和列表里的冲突

EventPointListEditorClass::EventPointListEditorClass(QWidget* parent, ConfigManager& cfg)
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

    QPushButton* addButton = new QPushButton(tr("添加事件点"), this);
    connect(addButton, &QPushButton::clicked, this, [this] {
        // ID 用现有最大 ID + 1；时间默认是 10 分钟后的整分钟
        unsigned id = 0;
        for (const EventPoint& item : config.event_point.event_point_list)
            id = qMax(id, item.id() + 1);
        const QTime time = QTime::currentTime().addSecs(600);
        config.event_point.event_point_list.append(EventPoint(id, QTime(time.hour(), time.minute()), tr("新事件点")));
        config.write();
        scheduleRebuild();
        });
    layout->addWidget(addButton);

    rebuild();
}

void EventPointListEditorClass::scheduleRebuild()
{
    QTimer::singleShot(0, this, &EventPointListEditorClass::rebuild);
}

void EventPointListEditorClass::updateEventPoint(int index, const QTime& time, const QString& name, int advanceTime, unsigned flashTimes)
{
    if (index >= config.event_point.event_point_list.size())
        return;
    EventPoint& item = config.event_point.event_point_list[index];
    const bool showing = item.isShowing();
    item = EventPoint(item.id(), time, name, advanceTime, flashTimes);
    item.setShowing(showing);
    config.write();
}

void EventPointListEditorClass::preview(int index)
{
    if (index >= config.event_point.event_point_list.size())
        return;
    const EventPoint& item = config.event_point.event_point_list[index];
    // 和托盘菜单的事件点提醒测试一样：事件点设在 PREVIEW_SECONDS 秒后，倒数胶囊从 PREVIEW_SECONDS 开始
    const EventPoint previewEventPoint(PREVIEW_EVENT_POINT_ID, QTime::currentTime().addSecs(PREVIEW_SECONDS + 1), item.name(), PREVIEW_SECONDS, item.flashTimes());
    EventPointReminder* reminder = new EventPointReminder(nullptr, previewEventPoint);
    reminder->setAttribute(Qt::WA_DeleteOnClose);
    // 用胶囊自己作为连接的上下文：预览中途关掉设置中心也能播完
    const QList<EventPoint> eventPointList = config.event_point.event_point_list;
    connect(reminder, &EventPointReminder::reached, reminder, [eventPointList](const EventPoint& eventPoint, const QRect& capsuleGeometry) {
        EventPointFullscreenReminder* fullscreenReminder = new EventPointFullscreenReminder(eventPoint, eventPointList, capsuleGeometry, eventPoint.flashTimes());
        fullscreenReminder->start();
        });
    reminder->show();
}

bool EventPointListEditorClass::eventFilter(QObject* watched, QEvent* event)
{
    // 没点进去的输入框不响应滚轮，滚轮用来滚动列表，免得滚动时误改了时间和数字
    if (event->type() == QEvent::Wheel && !static_cast<QWidget*>(watched)->hasFocus()) {
        QCoreApplication::sendEvent(ScrollArea->verticalScrollBar(), event);
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void EventPointListEditorClass::rebuild()
{
    while (QLayoutItem* item = RowsLayout->takeAt(0)) {
        if (QWidget* widget = item->widget())
            widget->deleteLater();
        delete item;
    }

    for (int i = 0; i < config.event_point.event_point_list.size(); i++) {
        const EventPoint& eventPoint = config.event_point.event_point_list[i];
        QWidget* row = new QWidget(RowsWidget);
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        QTimeEdit* timeEdit = new QTimeEdit(eventPoint.time(), row);
        timeEdit->setDisplayFormat("HH:mm:ss");
        QLineEdit* nameEdit = new QLineEdit(eventPoint.name(), row);
        nameEdit->setPlaceholderText(tr("事件名称"));
        nameEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[^;]*"), nameEdit)); // 分号是文件里的分隔符
        QSpinBox* advanceSpinBox = new QSpinBox(row);
        advanceSpinBox->setRange(1, 86399);
        advanceSpinBox->setValue(eventPoint.advanceTime());
        advanceSpinBox->setPrefix(tr("提前 ")); // 事件点提前多少秒开始倒数前缀文本
        advanceSpinBox->setSuffix(tr(" 秒")); // 事件点提前多少秒开始倒数后缀文本
        QSpinBox* flashSpinBox = new QSpinBox(row);
        flashSpinBox->setRange(0, USHRT_MAX);
        flashSpinBox->setValue(eventPoint.flashTimes());
        flashSpinBox->setPrefix(tr("闪烁 ")); // 事件点全屏提醒闪烁次数前缀文本
        flashSpinBox->setSuffix(tr(" 次")); // 事件点全屏提醒闪烁次数后缀文本
        QPushButton* previewButton = new QPushButton(tr("预览"), row);
        QPushButton* deleteButton = new QPushButton(tr("删除"), row);

        auto save = [this, i, timeEdit, nameEdit, advanceSpinBox, flashSpinBox] {
            updateEventPoint(i, timeEdit->time(), nameEdit->text(), advanceSpinBox->value(), flashSpinBox->value());
            };
        connect(timeEdit, &QTimeEdit::timeChanged, this, save);
        connect(nameEdit, &QLineEdit::textChanged, this, save);
        connect(advanceSpinBox, &QSpinBox::valueChanged, this, save);
        connect(flashSpinBox, &QSpinBox::valueChanged, this, save);
        connect(previewButton, &QPushButton::clicked, this, [this, i] { preview(i); });
        connect(deleteButton, &QPushButton::clicked, this, [this, i] {
            if (i < config.event_point.event_point_list.size()) {
                config.event_point.event_point_list.removeAt(i);
                config.write();
            }
            scheduleRebuild();
            });

        rowLayout->addWidget(timeEdit);
        rowLayout->addWidget(nameEdit, 1);
        rowLayout->addWidget(advanceSpinBox);
        rowLayout->addWidget(flashSpinBox);
        rowLayout->addWidget(previewButton);
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
