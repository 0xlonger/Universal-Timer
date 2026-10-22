#pragma once

#include <numbers>

#include <QString>
#include <QList>
#include <QPair>

inline constexpr double GOLDEN_RATIO_INV = 1.0 / std::numbers::phi; // 黄金比例倒数

inline int left_time = 0; // 剩余时间

inline const QList<QPair<QString, QString>> AVAILABLE_LANGUAGES = { { "zh-CN", "简体中文" } }; // 可以选的语言：代码和名称（名称用这种语言自己写）

inline const QString CURRENT_VERSION_STRING("2.3.0");

inline const QString GITHUB_RELEASE_URL("https://github.com/0xlonger/Universal-Timer/releases/");

inline const QString GITHUB_PAGES_DOMAIN_URL("https://0xlonger.github.io/");

inline const QString CLOUDFLARE_PAGES_DOMAIN_URL("https://lgr.pages.dev/");

inline const QString DOMAIN_URL("https://www.longer.xin/");