// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasmasetup

Kirigami.AbstractApplicationWindow {
    id: root

    title: i18n("Plasma Setup")

    visibility: Window.FullScreen

    // Disable the default shortcut for quitting the application.
    //
    // This is to prevent users from accidentally quitting the setup wizard
    // by pressing Ctrl+Q, which is the default shortcut for quitting Kirigami
    // applications.
    quitAction.shortcut: ""

    Wizard {
        anchors.fill: parent
    }
}
