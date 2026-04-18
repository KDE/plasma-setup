// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

/**
 * Handles keyboard layout choice.
 *
 * This class provides functionality to load available keyboard layouts,
 * set the current layout, and apply the selected layout.
 *
 * Layout choice applies to this application, as well as either the live session or
 * the newly created user depending on the context of how we are running.
 */
class KeyboardUtil : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /*!
     * The name of the currently selected keyboard layout, e.g. "us" or "ru".
     *
     * Can also be a comma-separated list of multiple layouts, in which case the first one
     * is expected to be a latin layout (e.g. "us") and the second one is expected to be a non-latin layout (e.g. "ru").
     *
     * Maps to the "X11Layout" value from systemd-localed.
     */
    Q_PROPERTY(QString layoutName READ layoutName WRITE setLayoutName NOTIFY layoutNameChanged)

    /*!
     * The variant of the currently selected keyboard layout, e.g. "intl" for the US
     * International layout.
     *
     * May be an empty string if no variant is set for the current layout. May be a
     * comma-separated list of multiple variants if there are multiple layouts, in which
     * case the first variant corresponds to the first layout, the second variant
     * corresponds to the second layout, and so on. Variants that are empty strings
     * correspond to layouts with no variant.
     *
     * Maps to the "X11Variant" value from systemd-localed.
     */
    Q_PROPERTY(QString layoutVariant READ layoutVariant WRITE setLayoutVariant NOTIFY layoutVariantChanged)

    /*!
     * The options of the currently selected keyboard layout, e.g. "grp:alt_shift_toggle" for
     * toggling between multiple layouts with Alt+Shift.
     *
     * May be an empty string if no options are set for the current layout.
     *
     * Maps to the "X11Options" value from systemd-localed.
     */
    Q_PROPERTY(QString layoutOptions READ layoutOptions WRITE setLayoutOptions NOTIFY layoutOptionsChanged)

public:
    explicit KeyboardUtil(QObject *parent = nullptr);

    QString layoutName() const;
    void setLayoutName(const QString &layoutName);

    QString layoutVariant() const;
    void setLayoutVariant(const QString &layoutVariant);

    QString layoutOptions() const;
    void setLayoutOptions(const QString &layoutOptions);

    /**
     * Applies the current keyboard layout choices to the current user and the system default.
     */
    Q_INVOKABLE void applyLayout();

Q_SIGNALS:
    void layoutNameChanged();
    void layoutVariantChanged();
    void layoutOptionsChanged();

private:
    /**
     * Applies the currently set keyboard layout for the current user.
     */
    void applyLayoutForCurrentUser();

    /**
     * Applies the currently set keyboard layout as the system default.
     */
    void applyLayoutAsSystemDefault();

    /**
     * Reads the current keyboard layout from the system and updates the properties accordingly.
     */
    void readCurrentKeyboardLayout();

    QString m_layoutName;
    QString m_layoutVariant;
    QString m_layoutOptions;
};
