/*
 * Universal-Timer
 * Copyright (c) 2025-2026 龙ger_longer
 * Author: 龙ger_longer
 * Developer: 龙ger_longer, new_pointer, XUESHENG_XSH
 * Version: 2.2.0
 * License: GNU General Public License v3.0
 * Repository: https://github.com/0xlonger/Universal-Timer
 * Description: A useful countdown timer application.
*/

#include "core/UniversalTimer2.h"
#include "core/LogManager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    defaultMessageHandler = qInstallMessageHandler(customMessageHandler);
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    UniversalTimer2 w;
    return a.exec();
}
