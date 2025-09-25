# SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

ecm_add_qml_module(plasmasetup_prepare
    URI "org.kde.plasmasetup.prepare.private"
    GENERATE_PLUGIN_SOURCE
)

kconfig_add_kcfg_files(plasmasetup_prepare colorssettings.kcfgc GENERATE_MOC)

target_sources(plasmasetup_prepare PRIVATE
    prepareutil.cpp
    prepareutil.h
)

target_link_libraries(plasmasetup_prepare PRIVATE
    Qt::Core
    Qt::DBus
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
    KF6::Screen
)

target_compile_definitions(plasmasetup_prepare PRIVATE -DTRANSLATION_DOMAIN=\"plasma-setup-prepare\")

ecm_finalize_qml_module(plasmasetup_prepare DESTINATION ${KDE_INSTALL_QMLDIR})
