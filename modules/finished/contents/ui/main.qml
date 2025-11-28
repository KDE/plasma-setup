// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.plasmasetup.components as PlasmaSetupComponents
import org.kde.plasmasetup

PlasmaSetupComponents.SetupModule {
    id: root

    nextEnabled: true

    contentItem: ColumnLayout {
        id: mainColumn

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            Label {
                id: finishedMessage
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                text: InitialStartUtil.finishedMessage()
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }

            Image {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Kirigami.Units.gridUnit
                Layout.maximumHeight: mainColumn.height - finishedMessage.height - Kirigami.Units.gridUnit
                fillMode: Image.PreserveAspectFit
                source: "konqi-calling.png"
            }
        }
    }
}
