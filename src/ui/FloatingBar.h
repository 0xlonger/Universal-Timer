#pragma once

#include "../core/ConfigManager.h"

#include <QLabel>
#include <QMargins>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QShowEvent>
#include <QPaintEvent>


class FloatingBarClass : public QWidget
{
    Q_OBJECT

public:
    FloatingBarClass(ConfigManager& cfg, QWidget* parent = nullptr);
    ~FloatingBarClass();

    QLabel* Bar;

    void applyTheme(unsigned radius, unsigned height); // 按当前主题设置悬浮条样式和阴影（还要用到配置里的位置和距顶边距离，改了它们也要调用）
    QMargins shadowMargins(FloatingBarPosition position, unsigned topMargin) const; // 阴影画在 Bar 外面，窗口四周要留出的位置（贴着屏幕边的一侧不留）
    void updateWindowFlags(); // 按配置更新窗口层级和点击穿透
    void updateOpacity(); // 按配置更新不透明度（平时 / 鼠标移入时淡化）
    void updateVisibility(); // 按“是否显示悬浮条”和隐藏规则显示或隐藏悬浮条
    void setShowFloatingBar(bool show); // 修改并保存“是否显示悬浮条”（设置中心的勾选框和托盘菜单的“隐藏悬浮条”是同一个开关）
    ForegroundWindowInfo lastForeignWindow() const { return m_lastForeignWindow; } // 最近一个不是万能倒计时自己的前台窗口

Q_SIGNALS:
    void hideRulesEvaluated(); // 每次判断完隐藏规则（编辑界面用来刷新满足状态）
    void showFloatingBarChanged(bool show); // “是否显示悬浮条”改了，另一处开关跟着更新

protected:

    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:

    ConfigManager& config;

    QGraphicsDropShadowEffect* m_shadowEffect; // 主题定义了 floating_bar_shadow_color 时显示的阴影
    QPropertyAnimation* m_opacityAnimation; // 不透明度渐变
    QTimer* m_mouseTimer; // 定时查询鼠标位置（开了点击穿透后窗口收不到鼠标事件）
    bool m_isMouseIn = false;
    bool m_isClickThroughPending = false; // 显示后还没有重新设置过点击穿透
    QTimer* m_hideRuleTimer; // 定时判断隐藏规则
    bool m_isHiddenByRule = false;
    ForegroundWindowInfo m_lastForeignWindow;

    void updateMouseStatus();
    void reapplyClickThrough();
    void evaluateHideRules();
};
