#pragma once

#include "core/ConfigManager.h"
#include "ui/FloatingBar.h"
#include "ui/DonatePage.h"
#include "ui/AboutPage.h"

#include <QStackedWidget>
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QResizeEvent>

class SettingsContentClass : public QStackedWidget
{
    Q_OBJECT

public:
    SettingsContentClass(QWidget* parent, ConfigManager& cfg, FloatingBarClass* bar);
    ~SettingsContentClass();

    enum class Page {
        None,
        GeneralSettingsPage,
        FloatingBarSettingsPage,
        ReminderSettingsPage,
        DonatePage,
        AboutPage
    };

    void setCurrentPage(const Page& page);

Q_SIGNALS:
    void clickedReminderPreviewButton();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:

    void applyTheme(); // 按当前主题和设置中心高度设置样式表

    void initializeObjects();
    void connectEmissions();

    // Config
    ConfigManager& config;

    // Floating Bar
    FloatingBarClass* FloatingBar;

    // Pages
    QWidget* GeneralSettingsPage;
    QWidget* FloatingBarSettingsPage;
    QWidget* ReminderSettingsPage;
    DonatePageClass* DonatePage;
    AboutPageClass* AboutPage;

    // Settings Items
    // General
    QDateTimeEdit* TargetDateTimeEdit;
    QComboBox* LanguageComboBox;
    QComboBox* ThemeComboBox;

    // Floating Bar
    QCheckBox* IsShowFloatingBarCheckBox;
    QLineEdit* FloatingBarTextLineEdit;
    QComboBox* FloatingBarLevelComboBox;
    QComboBox* FloatingBarPositionComboBox;
    QSpinBox* FloatingBarHeightSpinBox;
    QSpinBox* FloatingBarBorderRadiusSpinBox;
    QSpinBox* FloatingBarTopMarginSpinBox;
    QSpinBox* FloatingBarOpacitySpinBox;
    QCheckBox* IsMouseInFadingCheckBox;
    QCheckBox* IsMouseClickThroughCheckBox;

    // Reminder
    QCheckBox* IsShowReminderCheckBox;
    QLineEdit* ReminderTitleLineEdit;
    QLineEdit* ReminderTextLineEdit;
    QSpinBox* ReminderRemainingDaysToPlayCountdownSoundSpinBox;
    QSpinBox* ReminderRemainingDaysToPlayHeartbeatSoundSpinBox;
    QSpinBox* ReminderBlockShowTimesSpinBox;
    QPushButton* ReminderPreviewButton;


};