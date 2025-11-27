# SPDX-FileCopyrightText: (C) 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

configure_file(plasma-setup.service.in plasma-setup.service)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-setup.service
    DESTINATION ${KDE_INSTALL_SYSTEMDUNITDIR}/system
)

install(
    FILES "plasma-setup-polkit.rules"
    DESTINATION ${KDE_INSTALL_DATADIR}/polkit-1/rules.d
)

install(
    FILES "plasma-setup-sysuser.conf"
    DESTINATION  "lib/sysusers.d"
)

configure_file(plasma-setup-tmpfiles.conf.in plasma-setup-tmpfiles.conf)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-setup-tmpfiles.conf
    DESTINATION "lib/tmpfiles.d"
)

configure_file(plasma-setup.desktop.in plasma-setup.desktop)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-setup.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/plasma-setup
)

install(
    FILES plasmasetuprc
    DESTINATION ${KDE_INSTALL_FULL_CONFDIR}
)
