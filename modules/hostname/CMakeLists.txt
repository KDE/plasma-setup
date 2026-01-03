# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

ecm_add_qml_module(plasmasetup_hostname
    URI "org.kde.plasmasetup.hostname"
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES
        Qt::Core
)

ecm_finalize_qml_module(plasmasetup_hostname)
