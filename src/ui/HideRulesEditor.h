#pragma once

#include "core/ConfigManager.h"
#include "ui/FloatingBar.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

// 悬浮条隐藏规则集编辑窗口，布局参考 ClassIsland 的规则集编辑器：规则集 → 规则组 → 规则
class HideRulesEditorClass : public QDialog
{
    Q_OBJECT

public:
    HideRulesEditorClass(QWidget* parent, ConfigManager& cfg, FloatingBarClass* bar, const QFont& font);

private:

    ConfigManager& config;
    FloatingBarClass* FloatingBar;

    QWidget* GroupsWidget; // 放所有规则组的容器（在滚动区域里）
    QVBoxLayout* GroupsLayout;
    QPushButton* RulesetReversedButton;
    QComboBox* RulesetModeComboBox;
    QLabel* RulesetStateLabel;
    QLabel* ForegroundWindowLabel;
    QLabel* TitleLabel;
    QFont m_font;

    QList<QLabel*> GroupStateLabels;
    QList<QList<QLabel*>> RuleStateLabels;

    HideRuleset& ruleset() { return config.floating_bar.floating_bar_hide_rules; }

    void rebuild(); // 按规则集重新生成所有规则组和规则的控件
    void scheduleRebuild(); // 增删规则组 / 规则、改规则类型后调用（不能在控件自己的信号里直接删除它）
    void save();
    void refreshStates(); // 刷新满足状态和前台窗口信息
    void applyFont(); // 和设置中心一样给每个控件设置字体（标题除外）

    QWidget* createGroupWidget(int groupIndex);
    QWidget* createRuleWidget(int groupIndex, int ruleIndex);
    QWidget* createRuleSettingsWidget(int groupIndex, int ruleIndex);
    QComboBox* createComboBox(QWidget* parent, const QStringList& items, int currentIndex);
    QPushButton* createReversedButton(QWidget* parent, bool checked);
};
