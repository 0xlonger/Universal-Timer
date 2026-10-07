cmake_minimum_required(VERSION 3.16)
project(UniversalTimer2 LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
)
qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
    core/UniversalTimer2.h
    core/UniversalTimer2.cpp
    core/LogManager.h
    core/ConfigManager.h
    core/ConfigManager.cpp
    core/Global.h
    core/EventPoint.h
    core/HideRules.h
    core/HideRules.cpp
    core/ForegroundWindow.h
    core/ForegroundWindow.cpp
    core/ThemeManager.h
    core/ThemeManager.cpp
    ui/EventPointReminder.h
    ui/EventPointReminder.cpp
    ui/EventPointFullscreenReminder.h
    ui/EventPointFullscreenReminder.cpp
    ui/FloatingBar.h
    ui/FloatingBar.cpp
    ui/FullscreenPagesManager.h
    ui/FullscreenPagesManager.cpp
    ui/ReminderPage.h
    ui/ReminderPage.cpp
    ui/SettingsPage.h
    ui/SettingsPage.cpp
    ui/SettingsNav.h
    ui/SettingsNav.cpp
    ui/SettingsContent.h
    ui/SettingsContent.cpp
    ui/HideRulesEditor.h
    ui/HideRulesEditor.cpp
    ui/WelcomePage.h
    ui/WelcomePage.cpp
    ui/AboutPage.h
    ui/AboutPage.cpp
    ui/DonatePage.h
    ui/DonatePage.cpp
    ui/ScalableLabel.h
    ui/ScalableLabel.cpp
    platform/Resource.rc
    platform/resource.h
    ../assets/UniversalTimer2.qrc
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Multimedia
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

# 悬浮条隐藏规则在 Linux 上用 X11（libxcb）获取前台窗口；找不到 libxcb 时照常编译，只是前台窗口相关的规则不生效
if(UNIX AND NOT APPLE)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(XCB QUIET IMPORTED_TARGET xcb)
    endif()
    if(XCB_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::XCB)
        target_compile_definitions(${PROJECT_NAME} PRIVATE UT_HAVE_XCB)
    else()
        message(WARNING "没有找到 libxcb（Debian / Ubuntu 上可以安装 libxcb1-dev），悬浮条隐藏规则里和前台窗口有关的规则在 Linux 上不会生效")
    endif()
endif()
