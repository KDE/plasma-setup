# SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

ecm_add_qml_module(plasmasetup_prepareutil
    URI "org.kde.plasmasetup.prepareutil"
    GENERATE_PLUGIN_SOURCE
)

kconfig_add_kcfg_files(plasmasetup_prepareutil colorssettings.kcfgc GENERATE_MOC)

target_sources(plasmasetup_prepareutil PRIVATE
    prepareutil.cpp
    prepareutil.h
)

target_link_libraries(plasmasetup_prepareutil PRIVATE
    Qt::Core
    Qt::DBus
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
    KF6::Screen
)

target_compile_definitions(plasmasetup_prepareutil PRIVATE -DTRANSLATION_DOMAIN=\"plasma-setup-prepare\")

ecm_finalize_qml_module(plasmasetup_prepareutil)
