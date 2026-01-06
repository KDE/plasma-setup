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

    /*!
    * The message shown to users who already have an account on the system.
    */
    property string existingUserFinishedMessage: i18nc( // qmllint disable unqualified
        "%1 is the distro name",
        "Your device is now ready.<br /><br />Enjoy <b>%1</b>!",
        InitialStartUtil.distroName
    )

    /*!
    * The message shown to users who have just created a new account.
    */
    property string newUserFinishedMessage: i18nc( // qmllint disable unqualified
        "%1 is the distro name",
        "Your device is now ready.<br /><br />After clicking <b>Finish</b> you will be able to sign in to your new account.<br /><br />Enjoy <b>%1</b>!",
        InitialStartUtil.distroName
    )

    contentItem: ColumnLayout {
        id: mainColumn

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            Label {
                id: finishedMessage
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                text: AccountController.hasExistingUsers
                        ? root.existingUserFinishedMessage
                        : root.newUserFinishedMessage
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
