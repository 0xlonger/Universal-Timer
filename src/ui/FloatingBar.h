#pragma once

#include "../core/ConfigManager.h"

#include <QLabel>
#include <QMargins>
#include <QGraphicsDropShadowEffect>


class FloatingBarClass : public QWidget
{
    Q_OBJECT

public:
    FloatingBarClass(QWidget* parent = nullptr);
    ~FloatingBarClass();

    QLabel* Bar;

    void applyTheme(unsigned radius, unsigned height); // 按当前主题设置悬浮条样式和阴影
    QMargins shadowMargins(FloatingBarPosition position) const; // 阴影画在 Bar 外面，窗口四周要留出的位置（贴着屏幕边的一侧不留）

private:

    QGraphicsDropShadowEffect* m_shadowEffect; // 主题定义了 floating_bar_shadow_color 时显示的阴影
};
