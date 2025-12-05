<!--
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
    SPDX-License-Identifier: CC0-1.0
-->


# Creating Custom Modules for Plasma Setup

Plasma Setup uses a plugin-based page system that automatically discovers and
loads modules packaged as KPackages. This allows developers to extend the setup
wizard with custom pages without modifying the core application.

## Overview

The setup wizard loads all KPackages with the type `KDE/PlasmaSetup` and
displays them in order based on their weight values. Modules can provide:

- **QML UI**: Custom user interfaces using Qt Quick, Kirigami, and Plasma
  components
- **C++ Backends** (optional): Compiled code for heavy lifting or system
  integration (DBus, KConfig, KAuth, etc.)

The [`PagesModel`](../src/pagesmodel.cpp) automatically discovers modules at
runtime, sorts them by weight, and checks their `available` property to
determine which pages to show.

## Module Structure

A basic module requires the following structure:

```
<module-name>/
├── CMakeLists.txt
├── metadata.json
└── contents/
    └── ui/
        └── main.qml
```

### metadata.json

The metadata file defines the module's identity and position in the wizard:

```json
{
    "KPackageStructure": "KDE/PlasmaSetup",
    "KPlugin": {
        "Authors": [
            {
                "Email": "your@email.org",
                "Name": "Your Name"
            }
        ],
        "Description": "Description of what your module does",
        "Id": "org.kde.plasmasetup.yourmodule",
        "License": "LGPL-2.0-or-later",
        "Name": "Your Module Display Name"
    },
    "X-KDE-ParentApp": "org.kde.plasmasetup",
    "X-KDE-Weight": 30
}
```

#### X-KDE-Weight Ordering

The `X-KDE-Weight` field determines where your module appears in the wizard
sequence. Lower numbers appear earlier. Reference values from built-in modules:

- `0` - Language selection (first page)
- `5` - Keyboard layout
- `10` - Prepare (display scaling, theme)
- `20` - Account creation
- `40` - Time zone
- `50` - WiFi configuration
- `200` - Finished (final page)

Choose a weight value between existing modules to position your custom page
appropriately.

### contents/ui/main.qml

The main QML file must export a `SetupModule` component:

```qml
import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasmasetup.components as PlasmaSetupComponents

PlasmaSetupComponents.SetupModule {
    id: root
    
    // Control whether this page appears (default: true)
    available: someCondition
    
    // Control whether Next button is enabled (default: true)
    nextEnabled: validationPassed
    
    contentItem: ColumnLayout {
        // Your UI content here
        Kirigami.Heading {
            text: i18n("Your Page Title")
        }
    }
}
```

Key properties:
- **`available`**: Set to `false` to skip this page if not needed (e.g., skip
  WiFi if already connected)
- **`nextEnabled`**: Set to `false` to disable Next button until validation
  requirements are met
- **`contentItem`**: The actual page content to display

### CMakeLists.txt

For QML-only modules:

```cmake
kpackage_install_package(. org.kde.plasmasetup.yourmodule packages plasma)
```

The kpackage will end up installed under `/usr/share/plasma/packages/org.kde.plasmasetup.yourmodule/`.

## Adding C++ Backend Utilities

For modules needing compiled code, create a
separate utility directory alongside your module:

```
├── yourmodule/           # KPackage module
│   ├── CMakeLists.txt
│   ├── metadata.json
│   └── contents/ui/main.qml
└── yourmoduleutil/       # C++ utility (separate!)
    ├── CMakeLists.txt
    ├── yourutil.h
    └── yourutil.cpp
```

**Why separate?** The `kpackage_install_package()` command would include uncompiled
source files if they were in the same directory.

### Example CMakeLists.txt for C++ Utility

```cmake
ecm_add_qml_module(plasmasetup_yourutil
    URI "org.kde.plasmasetup.yourutil"
    GENERATE_PLUGIN_SOURCE
    SOURCES
        yourutil.cpp
        yourutil.h
)

target_link_libraries(plasmasetup_yourutil PRIVATE
    Qt::Core
    Qt::Qml
    KF6::I18n
)

ecm_finalize_qml_module(plasmasetup_yourutil)
```

## Installation

Modules are installed as KPackages with the type `KDE/PlasmaSetup`. For details
on package installation and management, see the
[KPackage Framework documentation](https://invent.kde.org/frameworks/kpackage).

## Getting Started

The best way to learn is by examining the existing modules in the
[`modules/`](../modules) directory. Start with simpler modules like
[`wifi/`](../modules/wifi) (QML-only) or [`language/`](../modules/language)
(with C++ backend) to understand the patterns.
