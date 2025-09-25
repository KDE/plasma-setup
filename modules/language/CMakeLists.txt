# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
# SPDX-License-Identifier: LGPL-2.1-or-later

ecm_qt_declare_logging_category(logging_SRCS
    HEADER "plasmasetup_language_debug.h"
    IDENTIFIER "PlasmaSetupLanguage"
    CATEGORY_NAME "org.kde.plasmasetup.language"
    DESCRIPTION "Plasma Setup Language Module"
    DEFAULT_SEVERITY Debug
    EXPORT PLASMASETUP_LANGUAGE
)

ecm_add_qml_module(plasmasetup_language
    URI "org.kde.plasmasetup.language"
    GENERATE_PLUGIN_SOURCE
    SOURCES
        languageutil.cpp
        ${logging_SRCS}
    DEPENDENCIES
        Qt::Core
)

target_link_libraries(plasmasetup_language PRIVATE
    Qt::Core
    Qt::DBus
    Qt::Qml
    KF6::I18n
)

ecm_finalize_qml_module(plasmasetup_language)
