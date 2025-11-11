/*
    SPDX-FileCopyrightText: 2022 Janet Blackquill <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import org.kde.kirigami as Kirigami

/*!
    A debouncer timer that can be used to delay actions until the user has stopped
    performing an action for a short period of time.
*/
Timer {
    property bool isTriggered: false

    function reset() {
        isTriggered = false;
        restart();
    }

    interval: Kirigami.Units.humanMoment

    onTriggered: {
        isTriggered = true;
    }
}
