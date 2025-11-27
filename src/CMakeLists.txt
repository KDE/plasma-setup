# SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_subdirectory(auth)
add_subdirectory(bootutil)
add_subdirectory(components)
add_subdirectory(packagestructure)

ecm_qt_declare_logging_category(logging_SRCS
    HEADER "plasmasetup_debug.h"
    IDENTIFIER "PlasmaSetup"
    CATEGORY_NAME "org.kde.plasmasetup"
    DESCRIPTION "Plasma Setup"
    EXPORT PLASMASETUP
)

add_executable(plasma-setup
    main.cpp
    pagesmodel.cpp
    pagesmodel.h
    accountcontroller.cpp
    accountcontroller.h
    displayutil.cpp
    displayutil.h
    initialstartutil.cpp
    initialstartutil.h
    keyboardutil.cpp
    keyboardutil.h
    ${plasmasetup_DBUS_SRCS}
    ${logging_SRCS}
)

ecm_add_qml_module(plasma-setup GENERATE_PLUGIN_SOURCE
    URI "org.kde.plasmasetup"
    DEPENDENCIES
        QtGui
        QtQuick
)

qt_target_qml_sources(plasma-setup
    QML_FILES
        qml/Main.qml
        qml/Wizard.qml
        qml/LandingComponent.qml
)

target_link_libraries(plasma-setup
    PUBLIC
        Qt::Widgets
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
        Qt::DBus
        Qt::Svg
        KF6::AuthCore
        KF6::ConfigCore
        KF6::I18n
        KF6::I18nQml
        KF6::Package
        KF6::ConfigGui
        PW::KWorkspace
        componentsplugin
        componentspluginplugin
)

target_include_directories(plasma-setup PRIVATE 
    ${CMAKE_BINARY_DIR} # Allow include of config-plasma-setup.h
)

install(TARGETS plasma-setup RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
