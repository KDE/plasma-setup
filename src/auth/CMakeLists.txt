# SPDX-FileCopyrightText: (C) 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

add_executable(plasma-setup-auth-helper
    authhelper.cpp
    authhelper.h
)

target_link_libraries(plasma-setup-auth-helper
    Qt::Core
    Qt::DBus
    KF6::AuthCore
    KF6::ConfigGui
    KF6::I18n
)

target_include_directories(plasma-setup-auth-helper PRIVATE
    ${CMAKE_BINARY_DIR} # Allow include of config-plasma-setup.h
)

# Enable exceptions for the PrivilegeGuard to ease error handling.
kde_target_enable_exceptions(plasma-setup-auth-helper PRIVATE)

kauth_install_actions(org.kde.plasmasetup org.kde.plasmasetup.actions)
install(TARGETS plasma-setup-auth-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(plasma-setup-auth-helper org.kde.plasmasetup root)
