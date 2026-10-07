#include "ui/HideRulesEditor.h"
#include "core/ThemeManager.h"

#include <QApplication>
#include <QScreen>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QFrame>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimeEdit>
#include <QListView>
#include <QTimer>

// 规则类型的显示名称，顺序和 HideRuleType 一致；前四个和 ClassIsland 的名称相同
static const QStringList RULE_TYPE_TEXTS = {
    QObject::tr("请选择规则"),
    QObject::tr("前台窗口标题"),
    QObject::tr("前台窗口类名"),
    QObject::tr("前台窗口进程"),
    QObject::tr("前台窗口状态是"),
    QObject::tr("当前时间在时间段内"),
    QObject::tr("今天是星期"),
    QObject::tr("距目标时间剩余天数"),
    QObject::tr("总是为真"),
    QObject::tr("总是为假"),
};

static QString stateText(HideRuleState state)
{
    switch (state) {
        case HideRuleState::Satisfied: return QObject::tr("● 满足");
        case HideRuleState::Unsatisfied: return QObject::tr("○ 不满足");
        default: return QObject::tr("－");
    }
}

static QString stateColor(HideRuleState state)
{
    switch (state) {
        case HideRuleState::Satisfied: return "rgb(120, 230, 120)";
        case HideRuleState::Unsatisfied: return "gray";
        default: return "gray";
    }
}

static void setStateLabel(QLabel* label, HideRuleState state)
{
    label->setText(stateText(state));
    label->setStyleSheet("color: " + stateColor(state) + ";");
}

HideRulesEditorClass::HideRulesEditorClass(QWidget* parent, ConfigManager& cfg, FloatingBarClass* bar, const QFont& font)
    : QDialog(parent), config(cfg), FloatingBar(bar), m_font(font)
{
    this->setObjectName("HideRulesEditor");
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setModal(true);
    // 输入框等控件沿用设置中心的样式表（从父控件继承），窗口背景、规则组边框等在主题的 HideRulesEditor 区段
    this->setStyleSheet(ThemeManager::instance().style("HideRulesEditor"));

    const QRect desktop = QApplication::primaryScreen()->geometry();
    this->resize(desktop.width() * 0.7, desktop.height() * 0.8);
    this->move(desktop.x() + (desktop.width() - this->width()) / 2, desktop.y() + (desktop.height() - this->height()) / 2);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(25, 25, 25, 25);

    TitleLabel = new QLabel(tr("悬浮条隐藏规则集"), this);
    TitleLabel->setObjectName("HideRulesEditorTitle");
    layout->addWidget(TitleLabel);
    layout->addWidget(new QLabel(tr("满足下面的规则集时隐藏悬浮条。"), this));

    ForegroundWindowLabel = new QLabel(this);
    ForegroundWindowLabel->setWordWrap(true);
    ForegroundWindowLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(ForegroundWindowLabel);

    // 规则集：非 + 规则组任一 / 全部满足时 + 添加组
    QHBoxLayout* rulesetLayout = new QHBoxLayout;
    RulesetReversedButton = createReversedButton(this, ruleset().reversed);
    connect(RulesetReversedButton, &QPushButton::toggled, this, [this](bool checked) {
        ruleset().reversed = checked;
        save();
        });
    RulesetModeComboBox = createComboBox(this, { tr("规则组任一满足时"), tr("规则组全部满足时") }, ruleset().mode == HideRuleLogicalMode::And ? 1 : 0);
    connect(RulesetModeComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        ruleset().mode = index == 1 ? HideRuleLogicalMode::And : HideRuleLogicalMode::Or;
        save();
        });
    QPushButton* addGroupButton = new QPushButton(tr("添加组"), this);
    connect(addGroupButton, &QPushButton::clicked, this, [this] {
        ruleset().groups.append(HideRuleGroup());
        save();
        scheduleRebuild();
        });
    RulesetStateLabel = new QLabel(this);
    rulesetLayout->addWidget(RulesetReversedButton);
    rulesetLayout->addWidget(RulesetModeComboBox);
    rulesetLayout->addWidget(addGroupButton);
    rulesetLayout->addStretch();
    rulesetLayout->addWidget(RulesetStateLabel);
    layout->addLayout(rulesetLayout);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GroupsWidget = new QWidget(scrollArea);
    GroupsWidget->setObjectName("HideRuleGroups");
    GroupsLayout = new QVBoxLayout(GroupsWidget);
    GroupsLayout->setContentsMargins(0, 0, 0, 0);
    scrollArea->setWidget(GroupsWidget);
    layout->addWidget(scrollArea, 1);

    QPushButton* doneButton = new QPushButton(tr("完成"), this);
    connect(doneButton, &QPushButton::clicked, this, &QDialog::accept);
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(doneButton);
    layout->addLayout(bottomLayout);

    connect(FloatingBar, &FloatingBarClass::hideRulesEvaluated, this, &HideRulesEditorClass::refreshStates);

    rebuild();
}

QComboBox* HideRulesEditorClass::createComboBox(QWidget* parent, const QStringList& items, int currentIndex)
{
    QComboBox* comboBox = new QComboBox(parent);
    QListView* listView = new QListView(comboBox); // 和设置中心一样，换成 QListView 下拉列表的样式表才生效
    listView->setAutoFillBackground(false);
    comboBox->setView(listView);
    comboBox->addItems(items);
    comboBox->setCurrentIndex(currentIndex);
    return comboBox;
}

QPushButton* HideRulesEditorClass::createReversedButton(QWidget* parent, bool checked)
{
    QPushButton* button = new QPushButton(tr("非"), parent);
    button->setCheckable(true);
    button->setChecked(checked);
    button->setToolTip(tr("反转"));
    return button;
}

void HideRulesEditorClass::save()
{
    config.write();
}

void HideRulesEditorClass::scheduleRebuild()
{
    QTimer::singleShot(0, this, &HideRulesEditorClass::rebuild);
}

void HideRulesEditorClass::rebuild()
{
    while (QLayoutItem* item = GroupsLayout->takeAt(0)) {
        if (QWidget* widget = item->widget())
            widget->deleteLater();
        delete item;
    }
    GroupStateLabels.clear();
    RuleStateLabels.clear();

    for (int i = 0; i < ruleset().groups.size(); i++)
        GroupsLayout->addWidget(createGroupWidget(i));
    GroupsLayout->addStretch();
    applyFont();
    refreshStates();
}

void HideRulesEditorClass::applyFont()
{
    for (QWidget* child : this->findChildren<QWidget*>())
        child->setFont(m_font);
    QFont titleFont = m_font;
    titleFont.setPixelSize(m_font.pixelSize() * 1.5);
    TitleLabel->setFont(titleFont);
}

QWidget* HideRulesEditorClass::createGroupWidget(int groupIndex)
{
    HideRuleGroup& group = ruleset().groups[groupIndex];
    QFrame* frame = new QFrame(GroupsWidget);
    frame->setObjectName("HideRuleGroup");
    QVBoxLayout* layout = new QVBoxLayout(frame);

    // 规则组：启用 + 非 + 规则任一 / 全部满足时 …… 复制 删除
    QHBoxLayout* headerLayout = new QHBoxLayout;
    QCheckBox* enabledCheckBox = new QCheckBox(tr("启用"), frame);
    enabledCheckBox->setChecked(group.enabled);
    connect(enabledCheckBox, &QCheckBox::toggled, this, [this, groupIndex](bool checked) {
        ruleset().groups[groupIndex].enabled = checked;
        save();
        });
    QPushButton* reversedButton = createReversedButton(frame, group.reversed);
    connect(reversedButton, &QPushButton::toggled, this, [this, groupIndex](bool checked) {
        ruleset().groups[groupIndex].reversed = checked;
        save();
        });
    QComboBox* modeComboBox = createComboBox(frame, { tr("规则任一满足时"), tr("规则全部满足时") }, group.mode == HideRuleLogicalMode::And ? 1 : 0);
    connect(modeComboBox, &QComboBox::currentIndexChanged, this, [this, groupIndex](int index) {
        ruleset().groups[groupIndex].mode = index == 1 ? HideRuleLogicalMode::And : HideRuleLogicalMode::Or;
        save();
        });
    QLabel* stateLabel = new QLabel(frame);
    GroupStateLabels.append(stateLabel);
    QPushButton* copyButton = new QPushButton(tr("复制"), frame);
    connect(copyButton, &QPushButton::clicked, this, [this, groupIndex] {
        ruleset().groups.insert(groupIndex + 1, ruleset().groups[groupIndex]);
        save();
        scheduleRebuild();
        });
    QPushButton* deleteButton = new QPushButton(tr("删除"), frame);
    connect(deleteButton, &QPushButton::clicked, this, [this, groupIndex] {
        ruleset().groups.removeAt(groupIndex);
        save();
        scheduleRebuild();
        });
    headerLayout->addWidget(enabledCheckBox);
    headerLayout->addWidget(reversedButton);
    headerLayout->addWidget(modeComboBox);
    headerLayout->addStretch();
    headerLayout->addWidget(stateLabel);
    headerLayout->addWidget(copyButton);
    headerLayout->addWidget(deleteButton);
    layout->addLayout(headerLayout);

    RuleStateLabels.append(QList<QLabel*>());
    for (int i = 0; i < group.rules.size(); i++)
        layout->addWidget(createRuleWidget(groupIndex, i));

    QPushButton* addRuleButton = new QPushButton(tr("添加规则"), frame);
    connect(addRuleButton, &QPushButton::clicked, this, [this, groupIndex] {
        ruleset().groups[groupIndex].rules.append(HideRule());
        save();
        scheduleRebuild();
        });
    QHBoxLayout* addRuleLayout = new QHBoxLayout;
    addRuleLayout->addWidget(addRuleButton);
    addRuleLayout->addStretch();
    layout->addLayout(addRuleLayout);
    return frame;
}

QWidget* HideRulesEditorClass::createRuleWidget(int groupIndex, int ruleIndex)
{
    const HideRule& rule = ruleset().groups[groupIndex].rules[ruleIndex];
    QWidget* widget = new QWidget(GroupsWidget);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    // 规则：非 + 规则类型 + 规则设置 …… 状态 删除
    QPushButton* reversedButton = createReversedButton(widget, rule.reversed);
    connect(reversedButton, &QPushButton::toggled, this, [this, groupIndex, ruleIndex](bool checked) {
        ruleset().groups[groupIndex].rules[ruleIndex].reversed = checked;
        save();
        });
    QComboBox* typeComboBox = createComboBox(widget, RULE_TYPE_TEXTS, static_cast<int>(rule.type));
    connect(typeComboBox, &QComboBox::currentIndexChanged, this, [this, groupIndex, ruleIndex](int index) {
        ruleset().groups[groupIndex].rules[ruleIndex].type = static_cast<HideRuleType>(index);
        save();
        scheduleRebuild();
        });
    QLabel* stateLabel = new QLabel(widget);
    RuleStateLabels.last().append(stateLabel);
    QPushButton* deleteButton = new QPushButton(tr("删除"), widget);
    connect(deleteButton, &QPushButton::clicked, this, [this, groupIndex, ruleIndex] {
        ruleset().groups[groupIndex].rules.removeAt(ruleIndex);
        save();
        scheduleRebuild();
        });
    layout->addWidget(reversedButton);
    layout->addWidget(typeComboBox);
    layout->addWidget(createRuleSettingsWidget(groupIndex, ruleIndex), 1);
    layout->addWidget(stateLabel);
    layout->addWidget(deleteButton);
    return widget;
}

QWidget* HideRulesEditorClass::createRuleSettingsWidget(int groupIndex, int ruleIndex)
{
    const HideRule& rule = ruleset().groups[groupIndex].rules[ruleIndex];
    auto ruleRef = [this, groupIndex, ruleIndex]() -> HideRule& { return ruleset().groups[groupIndex].rules[ruleIndex]; };
    QWidget* widget = new QWidget(GroupsWidget);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    switch (rule.type) {
        case HideRuleType::WindowTitle:
        case HideRuleType::WindowClassName:
        case HideRuleType::WindowProcessName: {
            // 和 ClassIsland 一样：不勾“正则”时要完全相等
            QLineEdit* lineEdit = new QLineEdit(rule.text, widget);
            lineEdit->setPlaceholderText(tr("要匹配的文本（不勾“正则”时要完全相同）"));
            connect(lineEdit, &QLineEdit::textChanged, this, [this, ruleRef](const QString& text) {
                ruleRef().text = text;
                save();
                });
            QCheckBox* regexCheckBox = new QCheckBox(tr("正则"), widget);
            regexCheckBox->setChecked(rule.regex);
            connect(regexCheckBox, &QCheckBox::toggled, this, [this, ruleRef](bool checked) {
                ruleRef().regex = checked;
                save();
                });
            layout->addWidget(lineEdit, 1);
            layout->addWidget(regexCheckBox);
            break;
        }
        case HideRuleType::WindowStatus: {
            QComboBox* comboBox = createComboBox(widget, { tr("普通"), tr("最大化"), tr("最小化"), tr("全屏") }, static_cast<int>(rule.windowStatus));
            connect(comboBox, &QComboBox::currentIndexChanged, this, [this, ruleRef](int index) {
                ruleRef().windowStatus = static_cast<HideRuleWindowStatus>(index);
                save();
                });
            layout->addWidget(comboBox);
            layout->addStretch();
            break;
        }
        case HideRuleType::TimeRange: {
            QTimeEdit* startEdit = new QTimeEdit(rule.startTime, widget);
            startEdit->setDisplayFormat("HH:mm:ss");
            connect(startEdit, &QTimeEdit::timeChanged, this, [this, ruleRef](const QTime& time) {
                ruleRef().startTime = time;
                save();
                });
            QTimeEdit* endEdit = new QTimeEdit(rule.endTime, widget);
            endEdit->setDisplayFormat("HH:mm:ss");
            connect(endEdit, &QTimeEdit::timeChanged, this, [this, ruleRef](const QTime& time) {
                ruleRef().endTime = time;
                save();
                });
            layout->addWidget(new QLabel(tr("从"), widget));
            layout->addWidget(startEdit);
            layout->addWidget(new QLabel(tr("到"), widget));
            layout->addWidget(endEdit);
            layout->addStretch();
            break;
        }
        case HideRuleType::Weekday: {
            const QStringList days = { tr("一"), tr("二"), tr("三"), tr("四"), tr("五"), tr("六"), tr("日") };
            for (int i = 0; i < 7; i++) {
                QPushButton* dayButton = new QPushButton(days[i], widget);
                dayButton->setCheckable(true);
                dayButton->setChecked(rule.weekdays & (1u << i));
                connect(dayButton, &QPushButton::toggled, this, [this, ruleRef, i](bool checked) {
                    if (checked) ruleRef().weekdays |= 1u << i;
                    else ruleRef().weekdays &= ~(1u << i);
                    save();
                    });
                layout->addWidget(dayButton);
            }
            layout->addStretch();
            break;
        }
        case HideRuleType::RemainingDays: {
            QComboBox* compareComboBox = createComboBox(widget, { "≤", "≥", "=" }, static_cast<int>(rule.compare));
            connect(compareComboBox, &QComboBox::currentIndexChanged, this, [this, ruleRef](int index) {
                ruleRef().compare = static_cast<HideRuleCompare>(index);
                save();
                });
            QSpinBox* daysSpinBox = new QSpinBox(widget);
            daysSpinBox->setRange(-100000, 100000);
            daysSpinBox->setValue(rule.days);
            daysSpinBox->setSuffix(tr(" 天"));
            connect(daysSpinBox, &QSpinBox::valueChanged, this, [this, ruleRef](int value) {
                ruleRef().days = value;
                save();
                });
            layout->addWidget(compareComboBox);
            layout->addWidget(daysSpinBox);
            layout->addStretch();
            break;
        }
        default:
            layout->addStretch();
            break;
    }
    return widget;
}

void HideRulesEditorClass::refreshStates()
{
    const ForegroundWindowInfo window = FloatingBar->lastForeignWindow();
    if (window.valid)
        ForegroundWindowLabel->setText(tr("最近一个前台窗口（不算万能倒计时自己的）：标题“%1”，类名“%2”，进程“%3”%4")
            .arg(window.title, window.className, window.processName,
                 window.fullscreen ? tr("，全屏") : (window.maximized ? tr("，最大化") : (window.minimized ? tr("，最小化") : QString()))));
    else
        ForegroundWindowLabel->setText(tr("还没有取到前台窗口信息。切换到别的窗口后，这里会显示它的标题、类名和进程。"));

    const HideRuleset& rules = ruleset();
    setStateLabel(RulesetStateLabel, rules.state);
    if (rules.state == HideRuleState::Satisfied) RulesetStateLabel->setText(tr("● 满足，悬浮条已隐藏"));
    for (int i = 0; i < GroupStateLabels.size() && i < rules.groups.size(); i++) {
        setStateLabel(GroupStateLabels[i], rules.groups[i].state);
        for (int j = 0; j < RuleStateLabels[i].size() && j < rules.groups[i].rules.size(); j++)
            setStateLabel(RuleStateLabels[i][j], rules.groups[i].rules[j].state);
    }
}
