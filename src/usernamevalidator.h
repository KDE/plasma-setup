// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <KLocalizedString>

#include <QMetaType>
#include <QRegularExpression>
#include <QString>

/**
 * Shared username validation logic for Plasma Setup.
 */

namespace PlasmaSetupValidation
{
namespace Account
{
Q_NAMESPACE

/**
 * Possible results of username validation.
 */
enum class UsernameValidationResult {
    Valid,
    Empty,
    TooLong,
    InvalidCharacters,
};

Q_ENUM_NS(UsernameValidationResult)

/**
 * Maximum number of characters allowed in a username.
 */
constexpr int MAX_USERNAME_LENGTH = 32;

/**
 * Validate the provided username.
 *
 * @param username The username to validate.
 * @return The result of the validation.
 */
inline UsernameValidationResult validateUsername(const QString &username)
{
    const QString trimmed = username.trimmed();

    if (trimmed.isEmpty()) {
        return UsernameValidationResult::Empty;
    }

    if (trimmed.size() > MAX_USERNAME_LENGTH) {
        return UsernameValidationResult::TooLong;
    }

    /**
     * Regular expression pattern for valid usernames.
     *
     * Usernames must start with a letter (A-Z, a-z) or underscore (_),
     * followed by letters, digits (0-9), periods (.), underscores (_), or hyphens (-).
     */
    static const QRegularExpression usernamePattern(QStringLiteral("^[A-Za-z_][A-Za-z0-9_.-]*$"));

    if (!usernamePattern.match(trimmed).hasMatch()) {
        return UsernameValidationResult::InvalidCharacters;
    }

    return UsernameValidationResult::Valid;
}

/**
 * Convenience helper that answers whether the username is valid.
 */
inline bool isUsernameValid(const QString &username)
{
    return validateUsername(username) == UsernameValidationResult::Valid;
}

/**
 * Returns user-facing feedback for a validation result. When the username is valid,
 * the returned string is empty.
 */
inline QString usernameValidationMessage(UsernameValidationResult result)
{
    switch (result) {
    case UsernameValidationResult::Valid:
        return QString();
    case UsernameValidationResult::Empty:
        return i18nc("@info", "Username cannot be empty.");
    case UsernameValidationResult::TooLong:
        return i18nc("@info", "Username is too long (maximum 32 characters).");
    case UsernameValidationResult::InvalidCharacters:
        return i18nc("@info", "Usernames must start with a letter or underscore.\n\nThey may contain only letters, numbers, periods, underscores, or hyphens.");
    }

    return QString();
}

} // namespace Account
} // namespace PlasmaSetupValidation
