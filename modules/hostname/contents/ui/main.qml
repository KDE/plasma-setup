// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.plasmasetup
import org.kde.plasmasetup.components as PlasmaSetupComponents

import org.kde.plasmasetup.hostnameutil

/**
* Page for setting the system's hostname.
*/
PlasmaSetupComponents.SetupModule {
    id: root

    available: HostnameUtil.hostnameIsDefault()

    /**
    * Whether the user has modified the field.
    */
    property bool hasEdited: false

    /**
    * True while we are waiting for the debounce timer to fire.
    */
    property bool validationPending: false

    /**
    * Whether the hostname entered is valid.
    */
    property bool debouncedHostnameValid: false

    /**
    * Validation message for the hostname entered.
    *
    * If empty, the hostname is valid.
    */
    property string debouncedHostnameMessage: ""

    function onPageActivated() {
        // Disallow these default hostnames, as they would break networking for
        // user shares.
        if (HostnameUtil.hostname === "localhost" || HostnameUtil.hostname === "localhost.localdomain") {
            let username = AccountController.username;
            let prefix = username.length > 0 ? username : "plasma";
            hostnameField.text = prefix + "-pc";
        }
    }

    /**
    * Update the validation properties based on the current hostname field text.
    */
    function updateValidation() {
        debouncedHostnameValid = HostnameUtil.isHostnameValid(hostnameField.text);
        debouncedHostnameMessage = HostnameUtil.hostnameValidationMessage(hostnameField.text);
        validationPending = false;
    }

    Component.onCompleted: updateValidation()

    nextEnabled: !root.validationPending && root.debouncedHostnameValid

    contentItem: ScrollView {
        anchors.centerIn: parent
        width: Math.min(mainColumn.implicitWidth + (Kirigami.Units.gridUnit * 2), parent.width)
        height: Math.min(mainColumn.implicitHeight, parent.height)

        ColumnLayout {
            id: mainColumn

            spacing: Kirigami.Units.gridUnit

            Label {
                id: titleLabel
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                text: i18nc("@info:usagetip", "What should this device be called?") // qmllint disable unqualified
            }

            Kirigami.FormLayout {
                Layout.leftMargin: Kirigami.Units.gridUnit
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.fillWidth: true

                TextField {
                    id: hostnameField

                    Kirigami.FormData.label: i18nc("@label:textbox", "Hostname:") // qmllint disable unqualified

                    text: HostnameUtil.hostname
                    onTextChanged: {
                        root.hasEdited = true;
                        root.validationPending = true;
                        validationDebouncer.reset();
                    }

                    onEditingFinished: {
                        HostnameUtil.hostname = text;
                        // `onEditingFinished` means the user pressed Enter or
                        // moved focus away so we want to validate immediately
                        // rather than waiting for the debouncer.
                        validationDebouncer.stop();
                        root.updateValidation();
                    }
                }

                Kirigami.InlineMessage {
                    Layout.fillWidth: true
                    visible: root.hasEdited && !root.validationPending && root.debouncedHostnameMessage.length > 0
                    text: root.debouncedHostnameMessage
                    type: Kirigami.MessageType.Error
                }

                PlasmaSetupComponents.Debouncer {
                    id: validationDebouncer

                    onDebounced: root.updateValidation()
                }
            }
        }
    }
}
