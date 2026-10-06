#include "core/Global.h"
#include "ui/ReminderPage.h"
#include "core/ThemeManager.h"

#include <QTimer>
#include <QDateTime>

ReminderPageClass::ReminderPageClass(QWidget* parent, const ConfigManager& cfg)
    : QWidget(parent), config(cfg)
{

    assert(parent != nullptr && "parent cannot be nullptr");
    if (parent == nullptr) throw std::invalid_argument("parent cannot be nullptr");

    this->setStyleSheet(ThemeManager::instance().style("ReminderPage"));

// 在Debug下编译时
#ifdef QT_DEBUG
    this->setStyleSheet(this->styleSheet() + "QWidget {border: 1px solid red;}");
#endif
    
    // Sound
    CountdownSound = new QSoundEffect(this);
    CountdownSound->setSource(QUrl::fromLocalFile("./sounds/countdown.wav"));
    HeartbeatSound = new QSoundEffect(this);
    HeartbeatSound->setSource(QUrl::fromLocalFile("./sounds/heartbeat.wav"));
    

    initializeObjects();
    adjustObjects(parent, 1.0);
    

    for (int i = 0; i < 4; i++) {
        ReminderBlockLabels.append(new QLabel(parent)); // 全屏提醒块标签
        ReminderBlockLabels[i]->setStyleSheet(ThemeManager::instance().style("ReminderBlock"));
        ReminderBlockLabels[i]->hide();
    }
    ReminderBlockLabels[0]->setGeometry(parent->height() * 0.05, parent->height() * 0.05, parent->height() * 0.1, parent->height() * 0.1); // 左上
    ReminderBlockLabels[1]->setGeometry(parent->width() - parent->height() * 0.15, parent->height() * 0.05, parent->height() * 0.1, parent->height() * 0.1); // 右上
    ReminderBlockLabels[2]->setGeometry(parent->height() * 0.05, parent->height() * 0.85, parent->height() * 0.1, parent->height() * 0.1); // 左下
    ReminderBlockLabels[3]->setGeometry(parent->width() - parent->height() * 0.15, parent->height() * 0.85, parent->height() * 0.1, parent->height() * 0.1); // 右下

    QTimer::singleShot(1000, [this] { showBlocks(); });
}

ReminderPageClass::~ReminderPageClass()
{
    for (int i = 0; i < 4; i++) {
        ReminderBlockLabels[i]->deleteLater();
    }
}

void ReminderPageClass::initializeObjects() {
    TitleLabel = new QLabel(config.reminder.reminder_text, this); // 全屏提醒自定义标题标签
    ConjunctionLabel = new QLabel(tr("还剩"), this); // 全屏提醒剩余时间标签
    DaysLabel = new QLabel(tr("天", "Fullscreen Reminder"), this); // 全屏提醒剩余天数标签
    NumberLabel = new QLabel(QString::number(left_time / 86400), this); // 全屏提醒剩余天数数字标签
    TextLabel = new QLabel(config.reminder.reminder_small_text + QString::number(left_time / 86400) + " DAYS", this); // 全屏提醒小文本标签
    ColorLabel = new QLabel(this); // 全屏提醒颜色标签

    NumberLabel->setStyleSheet(ThemeManager::instance().style("ReminderNumber"));
    ColorLabel->setStyleSheet(ThemeManager::instance().style("ReminderColor"));
    
    TitleLabel->show();
    ConjunctionLabel->show();
    DaysLabel->show();
    NumberLabel->show();
    TextLabel->show();
    ColorLabel->show();

}

void ReminderPageClass::adjustObjects(const QWidget* parent, const qreal scale, const unsigned short depth) {
    font.setPixelSize(parent->height() * 0.2 * scale);
    TitleLabel->setFont(font);
    ConjunctionLabel->setFont(font);
    DaysLabel->setFont(font);
    font.setPixelSize(parent->height() * 0.5 * scale);
    NumberLabel->setFont(font);
    font.setPixelSize(parent->height() * 0.05 * scale);
    TextLabel->setFont(font);

    TitleLabel->adjustSize();
    ConjunctionLabel->adjustSize();
    DaysLabel->adjustSize();
    NumberLabel->adjustSize();
    TextLabel->adjustSize();
    ColorLabel->resize(parent->width() * 0.01 * scale, ConjunctionLabel->height());

    TitleLabel->move(0, 0);
    ConjunctionLabel->move(TitleLabel->width() - ConjunctionLabel->width(), TitleLabel->height());
    NumberLabel->move(TitleLabel->width(), ConjunctionLabel->y() + ConjunctionLabel->height() - NumberLabel->height() * 0.85);
    DaysLabel->move(NumberLabel->x() + NumberLabel->width(), ConjunctionLabel->y());
    TextLabel->move(ConjunctionLabel->x() + parent->height() * 0.04 * scale, ConjunctionLabel->y() + ConjunctionLabel->height());
    ColorLabel->move(ConjunctionLabel->x() - ColorLabel->width(), ConjunctionLabel->y() + TextLabel->height());
    this->resize((DaysLabel->x() + DaysLabel->width()) > (TextLabel->x() + TextLabel->width()) ? (DaysLabel->x() + DaysLabel->width()) : (TextLabel->x() + TextLabel->width()), TextLabel->y() + TextLabel->height());
    
    // 当超过屏幕范围时
    if (this->width() > (parent->width() + 2) && depth < 10) {
        adjustObjects(parent, (qreal)parent->width() / (qreal)this->width(), depth + 1);
    }
}

void ReminderPageClass::showBlocks(const unsigned short times) {
    if (config.reminder.block_show_times != 0) {
        if ((left_time / 86400) <= config.reminder.remaining_days_to_play_heartbeat_sound) {
            CountdownSound->play();
            if (times % 2) HeartbeatSound->play();
        }
        else if ((left_time / 86400) <= config.reminder.remaining_days_to_play_countdown_sound) CountdownSound->play();
        for (int i = 0; i < ReminderBlockLabels.size(); i++) {
            ReminderBlockLabels[i]->show();
        }
        QTimer::singleShot(500, this, [this, times] {
            for (int i = 0; i < ReminderBlockLabels.size(); i++) {
                ReminderBlockLabels[i]->hide();
            }
            QTimer::singleShot(500, this, [this, times] {
                if (times < config.reminder.block_show_times) {
                    showBlocks(times + 1);
                }
                else emit finished();
                });
            });
    }
    else QTimer::singleShot(3000, this, [this] { emit finished(); });
}