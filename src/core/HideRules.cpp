#include "core/HideRules.h"

#include <QJsonArray>
#include <QRegularExpression>

// 规则类型在 JSON 里的名字
static const QList<QPair<HideRuleType, QString>> RULE_TYPE_NAMES = {
    { HideRuleType::None, "none" },
    { HideRuleType::WindowTitle, "window_title" },
    { HideRuleType::WindowClassName, "window_class_name" },
    { HideRuleType::WindowProcessName, "window_process_name" },
    { HideRuleType::WindowStatus, "window_status" },
    { HideRuleType::TimeRange, "time_range" },
    { HideRuleType::Weekday, "weekday" },
    { HideRuleType::RemainingDays, "remaining_days" },
    { HideRuleType::AlwaysTrue, "always_true" },
    { HideRuleType::AlwaysFalse, "always_false" },
};

static QString ruleTypeToString(HideRuleType type)
{
    for (const auto& [t, name] : RULE_TYPE_NAMES)
        if (t == type) return name;
    return "none";
}

static HideRuleType ruleTypeFromString(const QString& name)
{
    for (const auto& [t, n] : RULE_TYPE_NAMES)
        if (n == name) return t;
    return HideRuleType::None;
}

static QString modeToString(HideRuleLogicalMode mode) { return mode == HideRuleLogicalMode::And ? "and" : "or"; }
static HideRuleLogicalMode modeFromString(const QString& name, HideRuleLogicalMode fallback)
{
    if (name == "and") return HideRuleLogicalMode::And;
    if (name == "or") return HideRuleLogicalMode::Or;
    return fallback;
}

QJsonObject HideRule::toJson() const
{
    QJsonObject json;
    json["type"] = ruleTypeToString(type);
    json["reversed"] = reversed;
    switch (type) {
        case HideRuleType::WindowTitle:
        case HideRuleType::WindowClassName:
        case HideRuleType::WindowProcessName:
            json["text"] = text;
            json["regex"] = regex;
            break;
        case HideRuleType::WindowStatus:
            json["window_status"] = static_cast<int>(windowStatus);
            break;
        case HideRuleType::TimeRange:
            json["start"] = startTime.toString("HH:mm:ss");
            json["end"] = endTime.toString("HH:mm:ss");
            break;
        case HideRuleType::Weekday: {
            QJsonArray array;
            for (int i = 0; i < 7; i++)
                if (weekdays & (1u << i)) array.append(i + 1);
            json["weekdays"] = array;
            break;
        }
        case HideRuleType::RemainingDays:
            json["compare"] = compare == HideRuleCompare::LessOrEqual ? "<=" : (compare == HideRuleCompare::GreaterOrEqual ? ">=" : "=");
            json["days"] = days;
            break;
        default:
            break;
    }
    return json;
}

HideRule HideRule::fromJson(const QJsonObject& json)
{
    HideRule rule;
    rule.type = ruleTypeFromString(json["type"].toString());
    rule.reversed = json["reversed"].toBool(false);
    rule.text = json["text"].toString();
    rule.regex = json["regex"].toBool(false);
    rule.windowStatus = static_cast<HideRuleWindowStatus>(qBound(0, json["window_status"].toInt(static_cast<int>(HideRuleWindowStatus::Maximized)), 3));
    if (json.contains("start")) rule.startTime = QTime::fromString(json["start"].toString(), "HH:mm:ss");
    if (json.contains("end")) rule.endTime = QTime::fromString(json["end"].toString(), "HH:mm:ss");
    if (!rule.startTime.isValid()) rule.startTime = QTime(8, 0);
    if (!rule.endTime.isValid()) rule.endTime = QTime(17, 0);
    if (json.contains("weekdays")) {
        rule.weekdays = 0;
        for (const QJsonValue& value : json["weekdays"].toArray()) {
            const int day = value.toInt();
            if (day >= 1 && day <= 7) rule.weekdays |= 1u << (day - 1);
        }
    }
    const QString compare = json["compare"].toString("<=");
    rule.compare = compare == ">=" ? HideRuleCompare::GreaterOrEqual : (compare == "=" ? HideRuleCompare::Equal : HideRuleCompare::LessOrEqual);
    rule.days = json["days"].toInt(0);
    return rule;
}

QJsonObject HideRuleGroup::toJson() const
{
    QJsonObject json;
    QJsonArray array;
    for (const HideRule& rule : rules) array.append(rule.toJson());
    json["rules"] = array;
    json["mode"] = modeToString(mode);
    json["reversed"] = reversed;
    json["enabled"] = enabled;
    return json;
}

HideRuleGroup HideRuleGroup::fromJson(const QJsonObject& json)
{
    HideRuleGroup group;
    group.rules.clear();
    for (const QJsonValue& value : json["rules"].toArray()) group.rules.append(HideRule::fromJson(value.toObject()));
    group.mode = modeFromString(json["mode"].toString(), HideRuleLogicalMode::And);
    group.reversed = json["reversed"].toBool(false);
    group.enabled = json["enabled"].toBool(true);
    return group;
}

QJsonObject HideRuleset::toJson() const
{
    QJsonObject json;
    QJsonArray array;
    for (const HideRuleGroup& group : groups) array.append(group.toJson());
    json["groups"] = array;
    json["mode"] = modeToString(mode);
    json["reversed"] = reversed;
    return json;
}

HideRuleset HideRuleset::fromJson(const QJsonObject& json)
{
    HideRuleset ruleset;
    ruleset.groups.clear();
    for (const QJsonValue& value : json["groups"].toArray()) ruleset.groups.append(HideRuleGroup::fromJson(value.toObject()));
    ruleset.mode = modeFromString(json["mode"].toString(), HideRuleLogicalMode::Or);
    ruleset.reversed = json["reversed"].toBool(false);
    return ruleset;
}

static HideRuleState toState(bool value) { return value ? HideRuleState::Satisfied : HideRuleState::Unsatisfied; }

static bool isStringMatching(const HideRule& rule, const QString& str)
{
    if (!rule.regex)
        return str == rule.text;
    const QRegularExpression expression(rule.text);
    return expression.isValid() && expression.match(str).hasMatch();
}

// 单条规则是否满足（不含反转）
static bool isRuleSatisfied(const HideRule& rule, const HideRuleContext& context)
{
    const ForegroundWindowInfo& window = context.window;
    switch (rule.type) {
        // 没有前台窗口（包括万能倒计时自己的窗口在前台）时，前台窗口相关的规则都不满足
        case HideRuleType::WindowTitle:
            return window.valid && isStringMatching(rule, window.title);
        case HideRuleType::WindowClassName:
            return window.valid && isStringMatching(rule, window.className);
        case HideRuleType::WindowProcessName:
            return window.valid && isStringMatching(rule, window.processName);
        case HideRuleType::WindowStatus:
            if (!window.valid)
                return false;
            // 与 ClassIsland 相同：普通 = 都不是，最大化 = 最大化但不是全屏
            switch (rule.windowStatus) {
                case HideRuleWindowStatus::Normal: return !(window.fullscreen || window.maximized || window.minimized);
                case HideRuleWindowStatus::Maximized: return window.maximized && !window.fullscreen;
                case HideRuleWindowStatus::Minimized: return window.minimized;
                case HideRuleWindowStatus::Fullscreen: return window.fullscreen;
            }
            return false;
        case HideRuleType::TimeRange: {
            const QTime time = context.now.time();
            if (rule.startTime <= rule.endTime)
                return time >= rule.startTime && time < rule.endTime;
            return time >= rule.startTime || time < rule.endTime; // 跨过午夜
        }
        case HideRuleType::Weekday:
            return rule.weekdays & (1u << (context.now.date().dayOfWeek() - 1));
        case HideRuleType::RemainingDays:
            switch (rule.compare) {
                case HideRuleCompare::LessOrEqual: return context.remainingDays <= rule.days;
                case HideRuleCompare::GreaterOrEqual: return context.remainingDays >= rule.days;
                case HideRuleCompare::Equal: return context.remainingDays == rule.days;
            }
            return false;
        case HideRuleType::AlwaysTrue:
            return true;
        case HideRuleType::AlwaysFalse:
        case HideRuleType::None:
            return false;
    }
    return false;
}

// 规则组是否满足；组里没有选了类型的规则时返回 -1（不参与判断），和 ClassIsland 一样
static int isGroupSatisfied(HideRuleGroup& group, const HideRuleContext& context)
{
    bool hasRule = false;
    for (const HideRule& rule : group.rules)
        if (rule.type != HideRuleType::None) hasRule = true;
    if (!hasRule)
        return -1;

    bool satisfied = group.mode == HideRuleLogicalMode::And;
    for (HideRule& rule : group.rules) {
        if (rule.type == HideRuleType::None)
            continue;
        const bool result = isRuleSatisfied(rule, context) ^ rule.reversed;
        rule.state = toState(result);
        if (!result && group.mode == HideRuleLogicalMode::And) {
            satisfied = false;
            break;
        }
        if (result && group.mode == HideRuleLogicalMode::Or) {
            satisfied = true;
            break;
        }
    }
    return (satisfied ^ group.reversed) ? 1 : 0;
}

bool evaluateHideRuleset(HideRuleset& ruleset, const HideRuleContext& context)
{
    for (HideRuleGroup& group : ruleset.groups) {
        group.state = HideRuleState::Unknown;
        for (HideRule& rule : group.rules)
            rule.state = HideRuleState::Unknown;
    }
    if (ruleset.groups.isEmpty()) {
        ruleset.state = HideRuleState::Unsatisfied;
        return false;
    }

    bool satisfied = ruleset.mode == HideRuleLogicalMode::And;
    for (HideRuleGroup& group : ruleset.groups) {
        if (!group.enabled)
            continue;
        const int result = isGroupSatisfied(group, context);
        if (result < 0)
            continue;
        group.state = toState(result == 1);
        if (result == 0 && ruleset.mode == HideRuleLogicalMode::And) {
            satisfied = false;
            break;
        }
        if (result == 1 && ruleset.mode == HideRuleLogicalMode::Or) {
            satisfied = true;
            break;
        }
    }
    satisfied ^= ruleset.reversed;
    ruleset.state = toState(satisfied);
    return satisfied;
}
