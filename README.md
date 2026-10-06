> This project is licensed under GPLv3 (see LICENSE file) and uses Qt 6.10.1 which is licensed under the GNU Lesser General Public License v3.
>
> Qt Source Code Offer:
> You can obtain the complete corresponding source code for the Qt libraries used in this project from the official Qt download archive: https://download.qt.io/archive/qt/6.10/6.10.1/single/ (e.g., qt-everywhere-src-6.10.1.tar.xz).
>
> Alternatively, for a period of three years after the last distribution of this software, I will provide the source code upon request via [create an Issue on this GitHub repository].

# 万能倒计时 Universal-Timer

![GPLv3 License](https://img.shields.io/badge/license-GPLv3-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6.10.1-green.svg)
![Version](https://img.shields.io/badge/version-2.2.0-orange)

**万能倒计时**（原项目名 UniversalTimer2）是一个基于 Qt 的强大且可定制的倒计时应用程序。它通过在悬浮条中显示剩余时间，并提供带有动画和音效的全屏提醒，帮助您追踪重要事件。

---

## Table of Contents / 目录

- [万能倒计时 Universal-Timer](#万能倒计时-universal-timer)
  - [Table of Contents / 目录](#table-of-contents--目录)
  - [Features / 功能特性](#features--功能特性)
  - [Screenshots / 截图](#screenshots--截图)
  - [Usage / 使用方法](#usage--使用方法)
    - [First Run / 首次运行](#first-run--首次运行)
    - [Tray Icon / 托盘图标](#tray-icon--托盘图标)
    - [Full-Screen Reminder / 全屏提醒](#full-screen-reminder--全屏提醒)
    - [Customization / 自定义](#customization--自定义)
  - [Configuration / 配置](#configuration--配置)
  - [License / 许可证](#license--许可证)
  - [Contributing / 贡献](#contributing--贡献)
  - [Contact / 联系方式](#contact--联系方式)
    - [龙ger\_longer](#龙ger_longer)

---


## Features / 功能特性

- **悬浮倒计时条** – 可选的置顶显示，支持自定义文本、大小和位置，剩余时间一目了然。
- **全屏提醒** – 在预定时间或手动触发时显示，伴有红色方块动画和声音提示，时刻提醒您的重要事件。
- **自定义文本** – 修改主文本、小文本以及倒计时格式，满足您的个性化需求。
- **声音提醒** – 当剩余天数低于阈值时播放倒计时或心跳音，更加有效地提醒您。
- **托盘图标** – 快速访问设置、刷新和退出，方便快捷。
- **配置持久化** – 所有设置保存在 `config.ini` 中，确保下次启动时自动加载，无需重新配置。您也可随时手动修改，自由定制。
- **定时显示全屏提醒** – 在一天中的预设时间自动显示全屏提醒，让您在过度放松时也能及时提醒自己。您可以随时在`reminder_time_list.txt`文件中添加或删除时间，实现灵活的提醒管理。
- **欢迎界面** – 首次使用时提供引导，帮助您快速上手，轻松开始。
- **事件点提醒** – 当到达事件点提醒提前秒数的时间后，在屏幕上悬浮显示一个事件点提醒胶囊。事件点倒计时结束后，将显示事件点全屏提醒，以华丽的动画提醒事件点的到达。

---

## Screenshots / 截图
 
*截图即将添加。*

---

## Usage / 使用方法

### First Run / 首次运行

- 出现欢迎界面，点击 **开始** 继续。
- 默认日期为2025年6月30日，您可以在设置中随时更改。

### Tray Icon / 托盘图标

- 右键点击托盘图标，打开菜单：
  - **设置** – 打开全屏设置面板。
  - **刷新** – 从磁盘重新加载配置。
  - **退出** – 退出应用程序。

### Full-Screen Reminder / 全屏提醒

- `remember_time_list.txt` 文件中的时间会自动触发全屏提醒。
- 您也可以在设置面板中预览提醒。
- 当剩余天数低于阈值时，提醒会闪烁红色方块并播放声音。

### Customization / 自定义

- 从托盘菜单中打开 **设置**，调整以下内容：
  - 目标日期 / 时间
  - 悬浮条文本、高度、位置和置顶行为
  - 全屏提醒文本和触发阈值
  - 闪烁周期和声音选项
- 所有更改会立即保存。

---

## Configuration / 配置

- `config.ini` – 存储所有用户设置（INI 格式）。
- `reminder_time_list.txt` – 每行一个时间（HH:mm:ss）的文本文件，用于触发全屏提醒。
- `Universal-Timer.log` – 日志文件。
- `sound/` – 包含 `countdown.wav` 和 `heartbeat.wav` 的文件夹。

您可以手动编辑这些文件，但建议使用设置面板进行操作。

---

## License / 许可证

本项目采用 **GNU 通用公共许可证 v3.0** 授权。详情请参阅 [LICENSE](LICENSE) 文件。

---

## Contributing / 贡献

欢迎贡献！请提出 issue 或提交 pull request。

---

## Contact / 联系方式

### 龙ger_longer
- GitHub: [@longlonger2022](https://github.com/longlonger2022)
