# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

ecm_qt_declare_logging_category(logging_SRCS
    HEADER "plasmasetup_hostnameutil_debug.h"
    IDENTIFIER "PlasmaSetupHostnameUtil"
    CATEGORY_NAME "org.kde.plasmasetup.hostnameutil"
    DESCRIPTION "Plasma Setup Hostname Util Module"
    DEFAULT_SEVERITY Debug
    EXPORT PLASMASETUP_HOSTNAMEUTIL
)

qt_add_dbus_interface(hostname_DBUS_SRCS
    org.freedesktop.hostname1.xml
    hostname1_interface
)

ecm_add_qml_module(plasmasetup_hostnameutil
    URI "org.kde.plasmasetup.hostnameutil"
    GENERATE_PLUGIN_SOURCE
    SOURCES
        hostnameutil.cpp
        hostnameutil.h
        ${hostname_DBUS_SRCS}
        ${logging_SRCS}
)

target_link_libraries(plasmasetup_hostnameutil PRIVATE
    Qt::Core
    Qt::DBus
    Qt::Qml
    KF6::I18n
)

ecm_finalize_qml_module(plasmasetup_hostnameutil)
