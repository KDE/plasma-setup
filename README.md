<!--
    SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
    SPDX-License-Identifier: CC0-1.0
-->


# Plasma Setup

The Out-of-the-box (OOTB) experience that greets a user after system
installation or when starting up a new computer. Guides the user in creating the
system's first user account and configuring initial settings.


## Features

- User account creation
- Language selection
- Keyboard layout selection
- Time zone selection
- Network configuration


## Getting Started

> [!caution]
> It is not recommended to install this on your system — you should use a virtual machine instead. Installing this on real hardware will leave behind files not trivially uninstallable and could leave your system in a non-function state.

- Clone the repository:

```bash
git clone https://invent.kde.org/plasma/plasma-setup.git
```

- Build and install:

```bash
cmake -B build/
cmake --build build/ --parallel
sudo cmake --install build/
```

- Trigger system user creation:

```bash
sudo systemd-sysusers
```

- Enable the systemd service:

```bash
sudo systemctl enable plasma-setup.service
```

- Reboot:

With the systemd service enabled, reboot your system and the initial setup will run automatically.

### Completion flag file

When setup finishes successfully it creates a flag file to indicate this at
`/etc/plasma-setup-done`. The systemd unit checks for this flag and only runs if
it does not exist, so the out-of-box experience only runs once. If you
intentionally want to re-run the wizard (for example while developing Plasma
Setup), remove the file manually and reboot:

```bash
sudo rm /etc/plasma-setup-done
```

### Configuration

Plasma Setup can be customized via a system-wide configuration file located at
`/etc/xdg/plasmasetuprc`. This file allows administrators to set default values
and preferences, as well as control certain aspects of the setup process.

#### Build-time options

| Option            | Default           | Description                                                             |
|-------------------|-------------------|-------------------------------------------------------------------------|
| `LOGIN_DEFS_PATH` | `/etc/login.defs` | Path Plasma Setup reads to determine the minimum UID for regular users. |

Override any option at configure time, for example:

```bash
cmake -B build/ -DLOGIN_DEFS_PATH=/usr/lib/sysusers/login.defs
```

### Development Overrides

For development and testing purposes it may be useful to override some of the
behaviors of Plasma Setup. The following environment variables can be set to
modify the behavior:

- `PLASMA_SETUP_USER_CREATION_OVERRIDE=1`: Forces the account creation
  page to always be shown, whereas normally it would be skipped if existing
  users are detected.

### Creating Custom Modules (for Distributions/Administrators)

Plasma Setup supports extending the wizard with custom pages via KPackage
modules. These modules can provide QML user interfaces and optional C++
backends.

For a complete guide on creating custom modules, see
[`docs/CUSTOM_MODULES.md`](docs/CUSTOM_MODULES.md).

-----

The project is under active development and is not yet ready for production use.
Contributions and suggestions are very welcome.
