#pragma once

#include "ui/SettingsContent.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QResizeEvent>

class SettingsNavClass : public QWidget
{
    Q_OBJECT

public:
    SettingsNavClass(QWidget* parent, SettingsContentClass* content);
    ~SettingsNavClass();

Q_SIGNALS:
    void clickedCloseButton();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:

    void applyTheme(); // 按当前主题和导航栏高度设置样式表

    // Settings Content
    SettingsContentClass* SettingsContent;

    // Label
    QLabel* ChosenLabel;

    // PushButton
    QPushButton* GeneralSettingsButton;
    QPushButton* FloatingBarSettingsButton;
    QPushButton* ReminderSettingsButton;
    QPushButton* EventPointSettingsButton;
    QPushButton* DonateButton;
    QPushButton* AboutButton;
    QPushButton* CloseButton;

    // Animation
    QPropertyAnimation* ChosenLabelMoveAnimation;

};