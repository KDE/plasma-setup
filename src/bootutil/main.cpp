// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>

#include "bootutil.h"
#include "plasmasetup_bootutil_debug.h"

/**
    Small utility to determine if Plasma Setup should run at boot time, and to run it if necessary.

    This is intended to be run as a systemd service at boot time, before the display manager starts.

    This utility expects to be called one of the following ways:

    1. With the command line flag `--first-run`, which indicates that this is the first boot and
       Plasma Setup should run.

    These should be managed by the systemd service files.
 */
int main(int argc, char *argv[])
{
    qCInfo(PlasmaSetupBootUtil) << "Plasma Setup Boot Utility started.";

    QCoreApplication app(argc, argv);

    BootUtil bootUtil;

    const bool isFirstRun = app.arguments().contains(QStringLiteral("--first-run"));

    if (isFirstRun) {
        qCInfo(PlasmaSetupBootUtil) << "First boot detected. Running setup...";
        bootUtil.writeSDDMAutologin(true);
        return 0;
    }

    qCInfo(PlasmaSetupBootUtil) << "Boot check completed. No action needed.";

    // Exit the application
    return 0;
}
