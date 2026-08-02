// SPDX-FileCopyrightText: 2026 Tiziano Gaia <ti.gaia@proton.me>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "languagesortfilterproxymodel.h"

#include <QLocale>

LanguageSortFilterProxyModel::LanguageSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void LanguageSortFilterProxyModel::setFilterString(const QString &filter)
{
    if (filter == filterRegularExpression().pattern()) {
        return;
    }

    beginFilterChange();
    setFilterRegularExpression(QRegularExpression(QRegularExpression::escape(filter), QRegularExpression::CaseInsensitiveOption));
    endFilterChange();
}

int LanguageSortFilterProxyModel::rowForLanguage(const QString &language) const
{
    for (int row = 0; row < rowCount(); ++row) {
        const QModelIndex index = this->index(row, 0);

        if (data(index, LanguageCodeRole).toString() == language) {
            return row;
        }
    }

    return -1;
}

bool LanguageSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QRegularExpression regex = filterRegularExpression();

    if (regex.pattern().isEmpty()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString language = sourceModel()->data(index, Qt::DisplayRole).toString();

    const QLocale locale(language);
    const QString nativeName = locale.nativeLanguageName();
    const QString englishName = QLocale::languageToString(locale.language());

    return regex.match(language).hasMatch() || regex.match(nativeName).hasMatch() || regex.match(englishName).hasMatch();
}

QHash<int, QByteArray> LanguageSortFilterProxyModel::roleNames() const
{
    auto roles = QSortFilterProxyModel::roleNames();
    roles[LanguageCodeRole] = "languageCode";
    return roles;
}

QVariant LanguageSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == LanguageCodeRole) {
        return sourceModel()->data(mapToSource(index), Qt::DisplayRole);
    }

    return QSortFilterProxyModel::data(index, role);
}
