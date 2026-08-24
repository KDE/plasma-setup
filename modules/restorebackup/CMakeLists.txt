# SPDX-FileCopyrightText: Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
# SPDX-License-Identifier: LGPL-2.0-or-later

ecm_add_qml_module(plasmasetup_restorebackup
    URI "org.kde.plasmasetup.restorebackup"
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES
        Qt::Core
)

ecm_finalize_qml_module(plasmasetup_restorebackup)
