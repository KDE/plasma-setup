// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "bootutil.h"
#include "plasmasetup_bootutil_debug.h"

#include <QCoreApplication>
#include <QDebug>

/**
    Small utility that sets up the environment for Plasma Setup to run at boot time.

    Should be run early in the boot sequence, before the display manager starts.
 */
int main(int argc, char *argv[])
{
    qCInfo(PlasmaSetupBootUtil) << "Plasma Setup Boot Utility started.";

    QCoreApplication app(argc, argv);

    BootUtil bootUtil;

    bootUtil.writeSDDMAutologin(true);
    return 0;
}
