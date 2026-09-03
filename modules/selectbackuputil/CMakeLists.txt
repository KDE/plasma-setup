# SPDX-FileCopyrightText: 2026 Bharadwaj Raju <bharadwaj.raju@machinesoul.in>
# SPDX-License-Identifier: LGPL-2.0-or-later

ecm_add_qml_module(plasmasetup_selectbackuputil
    URI "org.kde.plasmasetup.selectbackuputil"
    GENERATE_PLUGIN_SOURCE
    SOURCES
        externaldrivebackupsmodel.cpp
        externaldrivebackupsmodel.h
)

target_link_libraries(plasmasetup_selectbackuputil PRIVATE
    Qt::Core
    Qt::Qml
    Qt::Concurrent
    KF6::Solid
    KF6::KIOCore
)

ecm_finalize_qml_module(plasmasetup_selectbackuputil)
