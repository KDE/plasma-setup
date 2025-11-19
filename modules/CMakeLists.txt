# SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

add_subdirectory(account)
kpackage_install_package(account org.kde.plasmasetup.account packages plasma)

# add_subdirectory(cellular)
# kpackage_install_package(cellular org.kde.plasmasetup.cellular packages plasma)

add_subdirectory(finished)
kpackage_install_package(finished org.kde.plasmasetup.finished packages plasma)

add_subdirectory(keyboard)
kpackage_install_package(keyboard org.kde.plasmasetup.keyboard packages plasma)

add_subdirectory(language)
kpackage_install_package(language org.kde.plasmasetup.language packages plasma)

add_subdirectory(prepare)
kpackage_install_package(prepare org.kde.plasmasetup.prepare packages plasma)

add_subdirectory(time)
kpackage_install_package(time org.kde.plasmasetup.time packages plasma)

add_subdirectory(wifi)
kpackage_install_package(wifi org.kde.plasmasetup.wifi packages plasma)


# ---------------------------- Helper Utilities ---------------------------- #

# Helper utilities should be kept separate from the KPackage modules, otherwise
# kpackage_install_package() will include all of the uncompiled source files.
add_subdirectory(languageutil)
add_subdirectory(prepareutil)
add_subdirectory(timeutil)
