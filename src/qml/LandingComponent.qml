// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasmasetup.prepareutil as Prepare
import org.kde.plasmasetup

Item {
    id: root

    readonly property real scaleStart: 1.4
    readonly property real scaleLanding: 1.2
    readonly property real scaleSteps: 1

    signal requestNextPage()

    function returnToLanding() {
        backgroundImage.scale = scaleLanding;
        contentOpacityAnim.to = 1;
        contentOpacityAnim.restart();
    }

    property real contentOpacity: 0
    NumberAnimation on contentOpacity {
        id: contentOpacityAnim
        running: true
        duration: 1000
        to: 1

        // shorten animation after initial run
        onFinished: duration = 200
    }

    Image {
        id: backgroundImage
        anchors.fill: parent

        readonly property bool isLandscape: width >= height

        source: {
            const landscapeFile = "5120x2880.png";
            const portraitFile  = "1440x2960.png";

            const lightFolder = "wallpapers/Next/contents/images/";
            const darkFolder  = "wallpapers/Next/contents/images_dark/";
            const themedFolder = Prepare.PrepareUtil.usingDarkTheme ? darkFolder : lightFolder;

            function locate(file) {
                return StandardPaths.locate(StandardPaths.GenericDataLocation, themedFolder + file)
                    || StandardPaths.locate(StandardPaths.GenericDataLocation, lightFolder + file);
            }

            const primary = isLandscape ? landscapeFile : portraitFile;
            const secondary = isLandscape ? portraitFile : landscapeFile;

            return locate(primary) || locate(secondary) || "";
        }
        fillMode: Image.PreserveAspectCrop

        opacity: 0

        NumberAnimation on opacity {
            running: true
            duration: 400
            to: 1
            easing.type: Easing.InOutQuad
        }

        // zoom animation
        scale: scaleStart
        Component.onCompleted: scale = scaleLanding

        Behavior on scale {
            NumberAnimation {
                duration: 2000
                easing.type: Easing.OutExpo
            }
        }

        // darken image slightly
        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.3)
        }
    }

    // split layout decisions into narrow vs short:
    // - isNarrow controls layout mode (desktop vs compact bottom bar)
    // - isShort only enables scrolling / tighter spacing, & keeps desktop layout if width is fine
    readonly property bool isNarrow: width < Kirigami.Units.gridUnit * 34
    readonly property bool isShort:  height < Kirigami.Units.gridUnit * 22
    readonly property bool isTight:  isNarrow || isShort

    ColumnLayout {
        anchors.fill: parent

        anchors.leftMargin:  root.isTight ? Kirigami.Units.gridUnit * 2 : Kirigami.Units.gridUnit * 4
        anchors.rightMargin: root.isTight ? Kirigami.Units.gridUnit * 2 : Kirigami.Units.gridUnit * 4
        anchors.bottomMargin: Kirigami.Units.gridUnit * 2
        spacing: Kirigami.Units.largeSpacing

        opacity: root.contentOpacity

        // spacers expand only on roomy screens to keep center content centered
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: !root.isTight
            Layout.preferredHeight: root.isTight ? Kirigami.Units.largeSpacing : 0
        }

        ColumnLayout {
            id: centerBlock
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing

            Label {
                Layout.fillWidth: true
                text: Kirigami.Settings.isMobile
                    ? i18n("Welcome to<br/><b>Plasma Mobile</b>")
                    : i18n("Welcome to<br/><b>Plasma Desktop</b>")

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap

                font.pointSize: 18
                color: "white"
            }

            Button {
                id: button
                Layout.alignment: Qt.AlignHCenter

                opacity: root.contentOpacity
                text: i18n("Begin Setup")
                icon.name: "plasma-symbolic"

                onClicked: {
                    backgroundImage.scale = scaleSteps;
                    contentOpacityAnim.to = 0;
                    contentOpacityAnim.restart();
                    root.requestNextPage()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: !root.isTight
            Layout.preferredHeight: root.isTight ? Kirigami.Units.largeSpacing : 0
        }

        // only switch to compact bottom bar when narrow, not merely short
        Item {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
            Layout.preferredHeight: root.isNarrow
                ? poweredBy.implicitHeight + Kirigami.Units.smallSpacing + sessionMenu.implicitHeight
                : Math.max(poweredBy.implicitHeight, sessionMenu.implicitHeight)

            Kirigami.Heading {
                id: poweredBy
                width: parent.width
                y: root.isNarrow ? 0 : (parent.height - height) / 2

                text: i18nc("%1 is the distro name", "Powered by<br/><b>%1</b>", InitialStartUtil.distroName)
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap

                level: 5
                color: "white"
            }

            SessionMenu {
                id: sessionMenu
                x: root.isNarrow ? (parent.width - width) / 2 : parent.width - width
                y: root.isNarrow ? poweredBy.height + Kirigami.Units.smallSpacing : (parent.height - height) / 2
            }
        }
    }
}
