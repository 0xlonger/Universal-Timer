#include "core/UniversalTimer2.h"
#include "core/LogManager.h"
#include "core/Global.h"
#include "ui/DonatePage.h"
#include "ui/EventPointReminder.h"
#include "ui/EventPointFullscreenReminder.h"

#include <QApplication>
#include <QScreen>
#include <QMenu>

UniversalTimer2::UniversalTimer2(QObject* parent)
    : QObject(parent)
{

    desktop = QApplication::primaryScreen()->geometry();

    // Floating Bar
    FloatingBar = new FloatingBarClass;

    // Fullscreen Pages
    FullscreenPages = new FullscreenPagesManager(nullptr, config, FloatingBar);
    FullscreenPages->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    FullscreenPages->resize(desktop.size());

    // About Page
    AboutPage = new AboutPageClass;
    AboutPage->resize(desktop.size() / 2);
    AboutPage->move((desktop.width() - AboutPage->width()) / 2, (desktop.height() - AboutPage->height()) / 2);

    // Tray Icon
    TrayIcon = new QSystemTrayIcon(QIcon(":/images/icons/Universal-Timer-2_icon.512px.png"), this); // 系统托盘图标
    TrayIcon->setToolTip(tr("万能倒计时"));
    TrayIcon->setContextMenu(new QMenu(tr("万能倒计时")));
    TrayIcon->contextMenu()->addAction(tr("设置中心"), FullscreenPages, &FullscreenPagesManager::showSettings); // 系统托盘菜单项：设置
    TrayIcon->contextMenu()->addAction(tr("赞助"), [this] {
        DonatePageClass* DonatePage = new DonatePageClass;
        DonatePage->setAttribute(Qt::WA_DeleteOnClose);
        DonatePage->resize(DonatePage->width(), desktop.height() / 2);
        DonatePage->move((desktop.width() - DonatePage->width()) / 2, (desktop.height() - DonatePage->height()) / 2);
        DonatePage->show();
        });
    TrayIcon->contextMenu()->addAction(tr("关于"), AboutPage, &AboutPageClass::showNormal);
    TrayIcon->contextMenu()->addSeparator();
    TrayIcon->contextMenu()->addAction(tr("刷新"), this, &UniversalTimer2::refresh); // 系统托盘菜单项：刷新
    TrayIcon->contextMenu()->addAction(tr("立即播报全屏提醒"), FullscreenPages, &FullscreenPagesManager::showReminder); // 系统托盘菜单项：立即播报全屏提醒
#ifdef QT_DEBUG
    TrayIcon->contextMenu()->addAction(tr("[DEBUG] 事件点提醒测试（提前15秒，闪烁4次）"), this, [this]() {
        EventPoint test_event_point(QTime::currentTime().addSecs(16), tr("测试"), 15, 4);
        EventPointReminder* event_point_reminder = new EventPointReminder(nullptr, test_event_point);
        event_point_reminder->setAttribute(Qt::WA_DeleteOnClose);
        connect(event_point_reminder, &EventPointReminder::reached, this, [this](const EventPoint& event_point, const QRect& capsule_geometry) {
            EventPointFullscreenReminder* event_point_fullscreen_reminder = new EventPointFullscreenReminder(event_point, config.event_point.event_point_list, capsule_geometry, event_point.flashTimes());
            event_point_fullscreen_reminder->start();
            });
        event_point_reminder->show();
        });
#endif
    TrayIcon->contextMenu()->addSeparator();
    TrayIcon->contextMenu()->addAction(tr("退出"), this, &qApp->quit); // 系统托盘菜单项：退出
    TrayIcon->show();

    refresh();

    // Connections
    timer.start(config.general.update_interval);
    connect(&timer, &QTimer::timeout, this, &UniversalTimer2::updateObjects);

}

UniversalTimer2::~UniversalTimer2()
{
    FloatingBar->deleteLater();
    FullscreenPages->deleteLater();
    AboutPage->deleteLater();
    TrayIcon->contextMenu()->deleteLater();
}


// 刷新函数
void UniversalTimer2::refresh() {

    qDebug() << "刷新……";

    left_time = QDateTime::currentDateTime().secsTo(config.general.target_date_time);

    if (!QFile::exists("config.ini")) {
        FullscreenPages->showWelcome();
        config.write();
    }
    else config.read();
    
    // Floating Bar
    if (!config.floating_bar.is_show_floating_bar) FloatingBar->hide();
    else FloatingBar->show();
    FloatingBar->Bar->setStyleSheet("background: rgba(255, 255, 255, 0.75); border-radius: " + QString::number(config.floating_bar.floating_bar_border_radius) + "px; color: red;"); // 更新悬浮条样式
    FloatingBar->setWindowFlags((config.floating_bar.floating_bar_on_top ? Qt::WindowStaysOnTopHint : Qt::WindowStaysOnBottomHint) | Qt::FramelessWindowHint | Qt::Tool);
    FloatingBar->setFixedHeight(config.floating_bar.floating_bar_height);
    QFont font;
    font.setPixelSize(config.floating_bar.floating_bar_height * GOLDEN_RATIO_INV);
    FloatingBar->Bar->setFont(font);

    // Reminder
    if (config.reminder.is_show_reminder) {
        FullscreenPages->showReminder();
    }

    qDebug() << "刷新结束";
}

// 更新函数
// 悬浮条更新函数
void UniversalTimer2::updateFloatingBar() {
    // 更新标签文本
    FloatingBar->Bar->setText(" " + config.floating_bar.floating_bar_text + QString::number(left_time / 86400) + tr("天", "Floating Bar") + QString::number((left_time % 86400) / 3600) + tr("时") + QString::number((left_time % 3600) / 60) + tr("分") + QString::number(left_time % 60) + tr("秒 "));

    // 更新大小
    FloatingBar->Bar->adjustSize();
    FloatingBar->Bar->resize(FloatingBar->Bar->width() + 20, config.floating_bar.floating_bar_height);
    FloatingBar->adjustSize();

    // 更新位置
    switch (config.floating_bar.floating_bar_position) {
        case FloatingBarPosition::TopCenter:
            FloatingBar->move((desktop.width() - FloatingBar->width()) / 2, 0);
            break;
        case FloatingBarPosition::TopRight:
            FloatingBar->move(desktop.width() - FloatingBar->width(), 0);
            break;
        case FloatingBarPosition::TopLeft:
            FloatingBar->move(0, 0);
            break;
    }
}

// 更新函数
void UniversalTimer2::updateObjects() {
    left_time = QDateTime::currentDateTime().secsTo(config.general.target_date_time);
    if (config.floating_bar.is_show_floating_bar) {
        updateFloatingBar();
    }
    // 定时显示全屏提醒
    if (config.reminder.is_show_reminder) {
        QTime current_time = QTime::currentTime();
        if (config.reminder.reminder_time_list.contains(QTime(current_time.hour(), current_time.minute(), current_time.second()))) {
            FullscreenPages->showReminder();
        }
    }
    // 定时显示事件点提醒
    for (const EventPoint& event_point_item : config.event_point.event_point_list) {
        QTime current_time = QTime::currentTime();
        if (QTime(current_time.hour(), current_time.minute(), current_time.second()) == event_point_item.time().addSecs(-event_point_item.advanceTime() - 1) && !event_point_item.isShowing()) {
            EventPointReminder* event_point_reminder = new EventPointReminder(nullptr, event_point_item);
            event_point_reminder->setAttribute(Qt::WA_DeleteOnClose);
            connect(event_point_reminder, &EventPointReminder::reached, this, [this](const EventPoint& event_point, const QRect& capsule_geometry) {
                EventPointFullscreenReminder* event_point_fullscreen_reminder = new EventPointFullscreenReminder(event_point, config.event_point.event_point_list, capsule_geometry, event_point.flashTimes());
                event_point_fullscreen_reminder->start();
                });
            event_point_reminder->show();
        }
    }
}