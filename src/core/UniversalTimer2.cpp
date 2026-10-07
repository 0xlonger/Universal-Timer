#include "core/UniversalTimer2.h"
#include "core/LogManager.h"
#include "core/Global.h"
#include "core/ThemeManager.h"
#include "ui/DonatePage.h"
#include "ui/EventPointReminder.h"
#include "ui/EventPointFullscreenReminder.h"

#include <QApplication>
#include <QProcess>
#include <QScreen>
#include <QMenu>

UniversalTimer2::UniversalTimer2(QObject* parent)
    : QObject(parent)
{

    desktop = QApplication::primaryScreen()->geometry();

    // Theme
    ThemeManager::exportBuiltInThemes(); // 第一次运行时把内置主题导出到 themes 文件夹，方便修改
    ThemeManager::instance().load(config.general.theme);

    // Floating Bar
    FloatingBar = new FloatingBarClass(config);
    connect(&ThemeManager::instance(), &ThemeManager::changed, this, [this] {
        FloatingBar->applyTheme(config.floating_bar.floating_bar_border_radius, config.floating_bar.floating_bar_height);
        updateFloatingBar();
        });

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
    TrayIcon->contextMenu()->addAction(tr("立即播报全屏提醒"), FullscreenPages, &FullscreenPagesManager::showReminder); // 系统托盘菜单项：立即播报全屏提醒
    // 在 Debug 下
    if (config.general.debug_mode) {
        TrayIcon->contextMenu()->addAction(tr("[DEBUG] 事件点提醒测试（提前15秒，闪烁4次）"), this, [this]() {
            EventPoint test_event_point(0x3f3f3f3f, QTime::currentTime().addSecs(16), tr("测试"), 15, 4);
            EventPointReminder* event_point_reminder = new EventPointReminder(nullptr, test_event_point);
            event_point_reminder->setAttribute(Qt::WA_DeleteOnClose);
            connect(event_point_reminder, &EventPointReminder::reached, this, [this](const EventPoint& event_point, const QRect& capsule_geometry) {
                EventPointFullscreenReminder* event_point_fullscreen_reminder = new EventPointFullscreenReminder(event_point, config.event_point.event_point_list, capsule_geometry, event_point.flashTimes());
                event_point_fullscreen_reminder->start();
                });
            event_point_reminder->show();
            });
    }
    TrayIcon->contextMenu()->addSeparator();
    TrayIcon->contextMenu()->addAction(tr("重启程序"), this, [] {
        QProcess::startDetached(qApp->applicationFilePath(), QStringList());
        qApp->quit();
        }); // 系统托盘菜单项：重启
    TrayIcon->contextMenu()->addAction(tr("刷新"), this, &UniversalTimer2::refresh); // 系统托盘菜单项：刷新
    TrayIcon->contextMenu()->addAction(tr("退出"), this, &qApp->quit); // 系统托盘菜单项：退出
    TrayIcon->show();

    refresh();

    // Connections
    timer.setSingleShot(true);
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer, &QTimer::timeout, this, [this] {
        updateObjects();
        scheduleNextUpdate();
        });
    scheduleNextUpdate();

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

    // Theme：重新读取主题文件，改了主题文件后点“刷新”即可生效
    ThemeManager::instance().load(config.general.theme);
    
    // Floating Bar
    FloatingBar->updateVisibility(); // 按“是否显示悬浮条”和隐藏规则显示或隐藏
    FloatingBar->applyTheme(config.floating_bar.floating_bar_border_radius, config.floating_bar.floating_bar_height); // 更新悬浮条样式
    FloatingBar->updateWindowFlags(); // 更新悬浮条层级和点击穿透
    FloatingBar->updateOpacity(); // 更新悬浮条不透明度和鼠标移入淡化
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
    // 主题有阴影时，窗口四周要给阴影留出位置，悬浮条本身的位置不变
    const QMargins margins = FloatingBar->shadowMargins(config.floating_bar.floating_bar_position, config.floating_bar.floating_bar_top_margin);
    FloatingBar->Bar->move(margins.left(), margins.top());
    FloatingBar->setFixedSize(FloatingBar->Bar->width() + margins.left() + margins.right(), FloatingBar->Bar->height() + margins.top() + margins.bottom());

    // 更新位置（距顶边距离是悬浮条本身离屏幕顶边的距离）
    const int y = int(config.floating_bar.floating_bar_top_margin) - margins.top();
    switch (config.floating_bar.floating_bar_position) {
        case FloatingBarPosition::TopCenter:
            FloatingBar->move((desktop.width() - FloatingBar->Bar->width()) / 2 - margins.left(), y);
            break;
        case FloatingBarPosition::TopRight:
            FloatingBar->move(desktop.width() - FloatingBar->Bar->width() - margins.left(), y);
            break;
        case FloatingBarPosition::TopLeft:
            FloatingBar->move(0, y);
            break;
    }
}

// 安排下一次更新
// 计时器的触发时刻和系统时间的整秒之间有一个固定的零头，零头恰好贴着整秒时，前后抖几毫秒就会落到整秒的另一边：
// 某一秒被跳过（悬浮条秒数跳、设在这一秒的定时全屏提醒和事件点不弹出），或者同一秒进来两次。
// 所以每次都安排在下一个整 update_interval 刚过 50ms 时触发
void UniversalTimer2::scheduleNextUpdate() {
    const int interval = qMax(1, int(config.general.update_interval));
    const int now = QTime::currentTime().msecsSinceStartOfDay();
    timer.start(interval - now % interval + 50);
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
    for (EventPoint& event_point_item : config.event_point.event_point_list) {
        QTime current_time = QTime::currentTime();
        if (QTime(current_time.hour(), current_time.minute(), current_time.second()) == event_point_item.time().addSecs(-event_point_item.advanceTime() - 1) && !event_point_item.isShowing()) {
            event_point_item.setShowing(true);
            EventPointReminder* event_point_reminder = new EventPointReminder(nullptr, event_point_item);
            event_point_reminder->setAttribute(Qt::WA_DeleteOnClose);
            connect(event_point_reminder, &EventPointReminder::reached, this, [this](const EventPoint& event_point, const QRect& capsule_geometry) {
                EventPointFullscreenReminder* event_point_fullscreen_reminder = new EventPointFullscreenReminder(event_point, config.event_point.event_point_list, capsule_geometry, event_point.flashTimes());
                connect(event_point_fullscreen_reminder, &EventPointFullscreenReminder::finished, this, [this](const EventPoint& event_point) {
                    for (EventPoint& event_point_item : config.event_point.event_point_list) {
                        if (event_point_item.id() == event_point.id()) {
                            event_point_item.setShowing(false);
                            break;
                        }
                    }
                });
                event_point_fullscreen_reminder->start();
            });
            event_point_reminder->show();
        }
    }
}