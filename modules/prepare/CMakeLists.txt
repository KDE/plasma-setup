# SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

ecm_add_qml_module(plasmasetup_prepare
    URI "org.kde.plasmasetup.prepare.private"
    GENERATE_PLUGIN_SOURCE
)

ecm_finalize_qml_module(plasmasetup_prepare)
