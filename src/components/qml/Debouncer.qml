/*
    SPDX-FileCopyrightText: 2022 Janet Blackquill <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

/*!
  A debounce timer for input validation and similar scenarios.

  Use to avoid performing expensive operations (such as validation) on every
  input change, and to avoid flickering validation messages while the user is
  still typing.

  The timer delays emitting the `debounced` signal until the interval has passed
  without the timer being reset. Call `reset()` whenever the input changes (for
  example, on every keystroke); validation or other expensive work is only performed
  after the user stops changing the input for the full duration of the interval.
*/
Timer {
    id: root

    property bool isTriggered: false

    signal debounced

    function reset() {
        isTriggered = false;
        restart();
    }

    interval: Kirigami.Units.humanMoment

    onTriggered: {
        isTriggered = true;
        debounced();
    }
}
