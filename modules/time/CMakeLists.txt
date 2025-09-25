# SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

ecm_add_qml_module(plasmasetup_time
    URI "org.kde.plasmasetup.time.private"
    GENERATE_PLUGIN_SOURCE
)

set_source_files_properties(org.freedesktop.timedate1.xml PROPERTIES NO_NAMESPACE TRUE)

qt_add_dbus_interface(plasmasetup_time_DBUS_SRCS
    org.freedesktop.timedate1.xml
    timedate_interface
)

target_sources(plasmasetup_time PRIVATE
    timeutil.cpp
    timeutil.h
    timezonemodel.cpp
    timezonemodel.h
    ${plasmasetup_time_DBUS_SRCS}
)

target_link_libraries(plasmasetup_time PUBLIC
    Qt::Core
    Qt::DBus
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
)

target_compile_definitions(plasmasetup_time PRIVATE -DTRANSLATION_DOMAIN=\"plasma-setup-time\")

ecm_finalize_qml_module(plasmasetup_time DESTINATION ${KDE_INSTALL_QMLDIR})
