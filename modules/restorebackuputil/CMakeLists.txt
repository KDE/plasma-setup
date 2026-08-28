# SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
# SPDX-License-Identifier: LGPL-2.0-or-later

ecm_add_qml_module(plasmasetup_restorebackuputil
    URI "org.kde.plasmasetup.restorebackuputil"
    GENERATE_PLUGIN_SOURCE
    SOURCES
        externaldrivesmodel.cpp
        externaldrivesmodel.h
)

target_link_libraries(plasmasetup_restorebackuputil PRIVATE
    Qt::Core
    Qt::Qml
    KF6::Solid
)

ecm_finalize_qml_module(plasmasetup_restorebackuputil)
