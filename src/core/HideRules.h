#pragma once

// 悬浮条隐藏规则集，结构和判断方式参考 ClassIsland 的规则集：规则集 → 规则组 → 规则

#include <QString>
#include <QTime>
#include <QDateTime>
#include <QList>
#include <QJsonObject>

// 前台窗口信息（由 ForegroundWindow 按平台获取）
struct ForegroundWindowInfo {
    bool valid = false; // 是否取到了前台窗口
    bool own = false; // 是不是万能倒计时自己的窗口（设置中心、全屏提醒等）
    QString title; // 标题
    QString className; // 类名（Linux 上是 WM_CLASS 的类名部分）
    QString processName; // 进程名（Windows 上不带 .exe）
    bool maximized = false; // 最大化
    bool fullscreen = false; // 全屏
    bool minimized = false; // 最小化
};

enum class HideRuleType {
    None, // 未选择（不参与判断）
    WindowTitle, // 前台窗口标题
    WindowClassName, // 前台窗口类名
    WindowProcessName, // 前台窗口进程
    WindowStatus, // 前台窗口状态是
    TimeRange, // 当前时间在时间段内
    Weekday, // 今天是星期
    RemainingDays, // 距目标时间剩余天数
    AlwaysTrue, // 总是为真
    AlwaysFalse // 总是为假
};

enum class HideRuleWindowStatus {
    Normal, // 普通
    Maximized, // 最大化
    Minimized, // 最小化
    Fullscreen // 全屏
};

enum class HideRuleCompare {
    LessOrEqual, // ≤
    GreaterOrEqual, // ≥
    Equal // =
};

// 满足状态，用于在编辑界面里显示
enum class HideRuleState {
    Unknown, // 未判断 / 不参与判断
    Unsatisfied, // 不满足
    Satisfied // 满足
};

struct HideRule {
    HideRuleType type = HideRuleType::None;
    bool reversed = false; // 反转（非）

    // 前台窗口标题 / 类名 / 进程：和 ClassIsland 一样，不用正则时要完全相等，用正则时只要能匹配上
    QString text;
    bool regex = false;

    HideRuleWindowStatus windowStatus = HideRuleWindowStatus::Maximized; // 前台窗口状态

    QTime startTime = QTime(8, 0); // 时间段开始（结束早于开始时表示跨过午夜）
    QTime endTime = QTime(17, 0); // 时间段结束

    unsigned weekdays = 0b0011111; // 星期：第 0 位是星期一 …… 第 6 位是星期日，默认周一到周五

    HideRuleCompare compare = HideRuleCompare::LessOrEqual; // 剩余天数比较方式
    int days = 0; // 剩余天数

    HideRuleState state = HideRuleState::Unknown;

    QJsonObject toJson() const;
    static HideRule fromJson(const QJsonObject& json);
};

enum class HideRuleLogicalMode {
    Or, // 任一满足
    And // 全部满足
};

struct HideRuleGroup {
    QList<HideRule> rules = { HideRule() };
    HideRuleLogicalMode mode = HideRuleLogicalMode::And; // 组内默认全部满足，和 ClassIsland 一样
    bool reversed = false;
    bool enabled = true;

    HideRuleState state = HideRuleState::Unknown;

    QJsonObject toJson() const;
    static HideRuleGroup fromJson(const QJsonObject& json);
};

struct HideRuleset {
    QList<HideRuleGroup> groups = { HideRuleGroup() };
    HideRuleLogicalMode mode = HideRuleLogicalMode::Or; // 组与组之间默认任一满足，和 ClassIsland 一样
    bool reversed = false;

    HideRuleState state = HideRuleState::Unknown;

    QJsonObject toJson() const;
    static HideRuleset fromJson(const QJsonObject& json);
};

// 判断时需要的当前状态
struct HideRuleContext {
    ForegroundWindowInfo window;
    QDateTime now;
    long long remainingDays = 0; // 距目标时间剩余天数（和悬浮条上显示的天数一致）
};

// 判断规则集是否满足，并把每个组和规则的满足状态写回去（供编辑界面显示）
bool evaluateHideRuleset(HideRuleset& ruleset, const HideRuleContext& context);
