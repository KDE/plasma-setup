# SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

ecm_add_qml_module(plasmasetup_timeutil
    URI "org.kde.plasmasetup.timeutil"
    GENERATE_PLUGIN_SOURCE
)

set_source_files_properties(org.freedesktop.timedate1.xml PROPERTIES NO_NAMESPACE TRUE)

qt_add_dbus_interface(plasmasetup_timeutil_DBUS_SRCS
    org.freedesktop.timedate1.xml
    timedate_interface
)

target_sources(plasmasetup_timeutil PRIVATE
    timeutil.cpp
    timeutil.h
    ${plasmasetup_timeutil_DBUS_SRCS}
)

target_link_libraries(plasmasetup_timeutil PUBLIC
    Qt::Core
    Qt::DBus
    Qt::Quick
    KF6::I18n
)

target_compile_definitions(plasmasetup_timeutil PRIVATE -DTRANSLATION_DOMAIN=\"plasma-setup-time\")

ecm_finalize_qml_module(plasmasetup_timeutil)
