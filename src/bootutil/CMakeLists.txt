# SPDX-FileCopyrightText: (C) 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

ecm_qt_declare_logging_category(logging_SRCS
    HEADER "plasmasetup_bootutil_debug.h"
    IDENTIFIER "PlasmaSetupBootUtil"
    CATEGORY_NAME "org.kde.plasmasetup.bootutil"
    DESCRIPTION "Plasma Setup Boot Utility"
    DEFAULT_SEVERITY Debug
    EXPORT PLASMASETUP_BOOTUTIL
)

add_executable(plasma-setup-bootutil
    bootutil.cpp
    bootutil.h
    main.cpp
    ${logging_SRCS}
)

target_link_libraries(plasma-setup-bootutil
    PRIVATE
        KF6::ConfigCore
        Qt::Core
)

install(TARGETS plasma-setup-bootutil RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
