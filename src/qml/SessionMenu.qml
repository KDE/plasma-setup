/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.plasma.private.kicker as Kicker
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels

/*!
    Provides system session actions such as restart and shutdown.

    A widget that displays buttons for system session actions like restart and shutdown. Intended to
    be placed in the landing page / outside of the wizard steps to allow users to easily manage the session.
*/
RowLayout {
    id: root

    spacing: Kirigami.Units.gridUnit

    Kicker.SystemModel {
        id: systemModel
    }

    KItemModels.KSortFilterProxyModel {
        id: filteredModel
        sourceModel: systemModel

        // Whitelist only restart + shutdown actions.
        // Note: Some setups use "reboot" instead of "restart", so accept both.
        readonly property var allowedActionIds: ({
            "restart": true,
            "reboot": true,
            "shutdown": true
        })

        filterRowCallback: (sourceRow, sourceParent) => {
            const FavoriteIdRole = sourceModel.KItemModels.KRoleNames.role("favoriteId");
            const favoriteId = String(sourceModel.data(sourceModel.index(sourceRow, 0, sourceParent), FavoriteIdRole));
            return allowedActionIds[favoriteId] === true;
        }

        /*!
            Triggers the action associated with the given proxy row.

            @param proxyRow The row index in the proxy model to trigger, e.g. reboot or shutdown.
        */
        function trigger(proxyRow: int): void {
            const sourceIndex = mapToSource(index(proxyRow, 0));
            systemModel.trigger(sourceIndex.row, "", null);
        }
    }

    Repeater {
        model: filteredModel

        delegate: ToolButton {
            required property int index
            required property var model

            text: model.display
            icon.name: model.decoration

            display: AbstractButton.TextBesideIcon
            flat: false

            ToolTip.text: text
            ToolTip.visible: hovered || activeFocus
            ToolTip.delay: Kirigami.Units.toolTipDelay

            onClicked: filteredModel.trigger(index)

            Keys.onEnterPressed: clicked()
            Keys.onReturnPressed: clicked()
        }
    }
}
