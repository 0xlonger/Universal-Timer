#include "core/Global.h"
#include "ui/SettingsNav.h"
#include "core/ThemeManager.h"

#include <QVBoxLayout>

SettingsNavClass::SettingsNavClass(QWidget* parent, SettingsContentClass* content)
    : QWidget(parent), SettingsContent(content)
{

    ChosenLabel = new QLabel(this);
    connect(&ThemeManager::instance(), &ThemeManager::changed, this, &SettingsNavClass::applyTheme);

    GeneralSettingsButton = new QPushButton(tr("总设置"), this);
    FloatingBarSettingsButton = new QPushButton(tr("悬浮条设置"), this);
    ReminderSettingsButton = new QPushButton(tr("全屏提醒设置"), this);
    DonateButton = new QPushButton(tr("赞助"), this);
    AboutButton = new QPushButton(tr("关于"), this);
    CloseButton = new QPushButton(tr("关闭设置"), this);

    ChosenLabelMoveAnimation = new QPropertyAnimation(ChosenLabel, "pos", this);
    ChosenLabelMoveAnimation->setDuration(500);
    ChosenLabelMoveAnimation->setEasingCurve(QEasingCurve::OutExpo);

    // Layout
    QVBoxLayout* Layout = new QVBoxLayout(this);
    Layout->addWidget(GeneralSettingsButton);
    Layout->addWidget(FloatingBarSettingsButton);
    Layout->addWidget(ReminderSettingsButton);
    Layout->addStretch();
    Layout->addWidget(DonateButton);
    Layout->addWidget(AboutButton);
    Layout->addWidget(CloseButton);
    Layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(Layout);

    connect(GeneralSettingsButton, &QPushButton::clicked, this, [this] {
        ChosenLabelMoveAnimation->setStartValue(ChosenLabel->pos());
        ChosenLabelMoveAnimation->setEndValue(GeneralSettingsButton->pos());
        ChosenLabelMoveAnimation->start();
        SettingsContent->setCurrentPage(SettingsContentClass::Page::GeneralSettingsPage);
        });
    connect(FloatingBarSettingsButton, &QPushButton::clicked, this, [this] {
        ChosenLabelMoveAnimation->setStartValue(ChosenLabel->pos());
        ChosenLabelMoveAnimation->setEndValue(FloatingBarSettingsButton->pos());
        ChosenLabelMoveAnimation->start();
        SettingsContent->setCurrentPage(SettingsContentClass::Page::FloatingBarSettingsPage);
        });
    connect(ReminderSettingsButton, &QPushButton::clicked, this,  [this] {
        ChosenLabelMoveAnimation->setStartValue(ChosenLabel->pos());
        ChosenLabelMoveAnimation->setEndValue(ReminderSettingsButton->pos());
        ChosenLabelMoveAnimation->start();
        SettingsContent->setCurrentPage(SettingsContentClass::Page::ReminderSettingsPage);
        });
    connect(DonateButton, &QPushButton::clicked, this, [this] {
        ChosenLabelMoveAnimation->setStartValue(ChosenLabel->pos());
        ChosenLabelMoveAnimation->setEndValue(DonateButton->pos());
        ChosenLabelMoveAnimation->start();
        SettingsContent->setCurrentPage(SettingsContentClass::Page::DonatePage);
        });
    connect(AboutButton, &QPushButton::clicked, this, [this] {
        ChosenLabelMoveAnimation->setStartValue(ChosenLabel->pos());
        ChosenLabelMoveAnimation->setEndValue(AboutButton->pos());
        ChosenLabelMoveAnimation->start();
        SettingsContent->setCurrentPage(SettingsContentClass::Page::AboutPage);
        });
    connect(CloseButton, &QPushButton::clicked, this, [this] {
        ChosenLabelMoveAnimation->setStartValue(ChosenLabel->pos());
        ChosenLabelMoveAnimation->setEndValue(CloseButton->pos());
        ChosenLabelMoveAnimation->start();
        SettingsContent->setCurrentPage(SettingsContentClass::Page::None);
        emit clickedCloseButton();
        });

}

SettingsNavClass::~SettingsNavClass()
{}

void SettingsNavClass::applyTheme() {
    // 尺寸按导航栏高度算，作为变量提供给主题
    this->setStyleSheet(ThemeManager::instance().style("SettingsNav", {
        { "nav_font_size", QString("%1px").arg(this->height() * 0.03) },
        { "nav_padding", QString("%1px").arg((this->height() * 0.03 / GOLDEN_RATIO_INV - this->height() * 0.03) / 2) },
        }));
    ChosenLabel->setStyleSheet(ThemeManager::instance().style("SettingsNavChosen"));
}

void SettingsNavClass::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    applyTheme();

    this->layout()->activate();

    ChosenLabel->setGeometry(CloseButton->geometry());


}