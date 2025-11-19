# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

ecm_qt_declare_logging_category(logging_SRCS
    HEADER "plasmasetup_languageutil_debug.h"
    IDENTIFIER "PlasmaSetupLanguageUtil"
    CATEGORY_NAME "org.kde.plasmasetup.languageutil"
    DESCRIPTION "Plasma Setup Language Util Module"
    DEFAULT_SEVERITY Debug
    EXPORT PLASMASETUP_LANGUAGEUTIL
)

ecm_add_qml_module(plasmasetup_languageutil
    URI "org.kde.plasmasetup.languageutil"
    GENERATE_PLUGIN_SOURCE
    SOURCES
        languageutil.cpp
        ${logging_SRCS}
)

target_link_libraries(plasmasetup_languageutil PRIVATE
    Qt::Core
    Qt::DBus
    Qt::Qml
    KF6::I18n
)

ecm_finalize_qml_module(plasmasetup_languageutil)
