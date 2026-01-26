// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasmasetup

Kirigami.AbstractApplicationWindow {
    id: root

    title: i18n("Plasma Setup")

    visibility: Window.FullScreen

    Wizard {
        anchors.fill: parent
    }
}
