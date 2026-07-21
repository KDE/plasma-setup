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
    if (m_filterString == filter) {
        return;
    }

    beginFilterChange();
    m_filterString = filter;
    endFilterChange();
}

bool LanguageSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterString.isEmpty()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString language = sourceModel()->data(index, Qt::DisplayRole).toString();

    const QLocale locale(language);

    const QString nativeName = locale.nativeLanguageName();
    const QString englishName = QLocale::languageToString(locale.language());

    return language.contains(m_filterString, Qt::CaseInsensitive) || nativeName.contains(m_filterString, Qt::CaseInsensitive)
        || englishName.contains(m_filterString, Qt::CaseInsensitive);
}

QHash<int, QByteArray> LanguageSortFilterProxyModel::roleNames() const
{
    auto roles = QSortFilterProxyModel::roleNames();
    roles[Qt::UserRole + 1] = "languageCode";
    return roles;
}

QVariant LanguageSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::UserRole + 1) {
        return sourceModel()->data(mapToSource(index), Qt::DisplayRole);
    }

    return QSortFilterProxyModel::data(index, role);
}
