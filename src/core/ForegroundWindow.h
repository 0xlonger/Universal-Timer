#pragma once

#include "core/HideRules.h"

// 获取前台窗口的标题、类名、进程名和状态。万能倒计时自己的窗口（设置中心、全屏提醒等）在前台时 own = true
// Windows 用 Win32 API；Linux 用 X11（需要编译时找到 libxcb，否则一直返回 valid = false）；其他平台一直返回 valid = false
ForegroundWindowInfo queryForegroundWindow();
