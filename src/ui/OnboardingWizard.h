#pragma once

#include "core/ConfigManager.h"
#include "ui/ReminderTimeListEditor.h"
#include "ui/EventPointListEditor.h"

#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>

// 第一次打开程序时的引导：欢迎页点“开始”后依次设置语言、主题、倒计时、定时全屏提醒和事件点
// 每一步改的都是设置中心里的同一个设置项，改了立即保存；换页时和打开设置中心一样，有色块从左往右扫过
class OnboardingWizardClass : public QWidget
{
    Q_OBJECT

public:
    OnboardingWizardClass(QWidget* parent, ConfigManager& cfg);

Q_SIGNALS:
    void finished(); // 点了最后一步的“完成”

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    ConfigManager& config;

    QStackedWidget* Pages; // 第 0 页是欢迎页，后面每一步一页
    QWidget* SlideWidget; // 换页时扫过的色块
    QPropertyAnimation* SlideInAnimation;
    QPropertyAnimation* SlideOutAnimation;
    QSequentialAnimationGroup* SlideAnimationGroup;
    int m_targetIndex = 0;
    bool m_isFinished = false;

    // 每一步的页面（用来在大小改变时调整边距、字号）
    struct Step {
        QWidget* page;
        QVBoxLayout* layout;
        QLabel* number; // 第几步
        QLabel* title;
        QList<QWidget*> forms; // 表单限制宽度，免得输入框拉得太长
        QList<QPushButton*> buttons;
    };
    QList<Step> m_steps;

    // 语言
    QLabel* LanguageLabel;
    QComboBox* LanguageComboBox;

    // 主题
    QComboBox* ThemeComboBox;

    // 倒计时
    QDateTimeEdit* TargetDateTimeEdit;
    QLineEdit* FloatingBarTextLineEdit;

    // 定时全屏提醒
    QCheckBox* IsShowReminderCheckBox;
    QWidget* ReminderSettingsWidget; // 不显示全屏提醒时变灰的部分
    QLineEdit* ReminderTitleLineEdit;
    QLineEdit* ReminderTextLineEdit;
    ReminderTimeListEditorClass* ReminderTimeListEditor;

    // 事件点
    EventPointListEditorClass* EventPointListEditor;

    // 悬浮条预览（主题和倒计时两步里各有一个）
    struct Preview {
        QLabel* bar;
        QGraphicsDropShadowEffect* shadow;
    };
    QList<Preview> m_previews;
    QTimer* PreviewTimer; // 每秒更新预览里的剩余时间

    void addStep(const QString& title, const QString& subtitle, QWidget* body, const QList<QWidget*>& forms);
    QWidget* createPreview();
    void updatePreviewText();
    void goTo(int index);
    void finish();
    void applyTheme();
};
