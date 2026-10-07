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
    * The message shown before the user confirms setup.
    */
    property string almostReadyMessage: xi18nc(
        "@info",
        "Your device is almost ready.<nl/><nl/>After clicking <interface>Confirm</interface>, the system will start being prepared."
    )

    /*!
    * The message shown to users who already have an account on the system.
    */
    property string existingUserFinishedMessage: i18nc(
        "%1 is the distro name",
        "Your device is now ready.<br /><br />Enjoy <b>%1</b>!",
        InitialStartUtil.distroName
    )

    /*!
    * The message shown to users who have just created a new account.
    */
    property string newUserFinishedMessage: i18nc(
        "%1 is the distro name",
        "Your device is now ready.<br /><br />After clicking <b>Finish</b> you will be able to sign in to your new account.<br /><br />Enjoy <b>%1</b>!",
        InitialStartUtil.distroName
    )

    contentItem: ColumnLayout {
        id: mainColumn

        ColumnLayout {
            spacing: Kirigami.Units.gridUnit
            Layout.alignment: Qt.AlignCenter

            Label {
                id: finishedMessage
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                visible: InitialStartUtil.state === InitialStartUtil.Finished || InitialStartUtil.state === InitialStartUtil.Asking
                text: {
                    if (InitialStartUtil.state === InitialStartUtil.Asking) {
                        return root.almostReadyMessage
                    }
                    return AccountController.hasExistingUsers
                        ? root.existingUserFinishedMessage
                        : root.newUserFinishedMessage
                }
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }

            ProgressBar {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 30
                Layout.alignment: Qt.AlignHCenter
                visible: InitialStartUtil.state === InitialStartUtil.Finishing
                indeterminate: true
            }

            Image {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                Layout.maximumHeight: mainColumn.height - finishedMessage.height - Kirigami.Units.gridUnit
                visible: InitialStartUtil.state === InitialStartUtil.Finished
                fillMode: Image.PreserveAspectFit
                source: "konqi-calling.png"
            }
        }
    }
}
