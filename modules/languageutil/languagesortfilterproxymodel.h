// SPDX-FileCopyrightText: 2026 Tiziano Gaia <ti.gaia@proton.me>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QSortFilterProxyModel>
#include <QString>

class QModelIndex;
class QVariant;

/**
 * A proxy model that filters the list of available languages
 * by locale code, native language name, and English language name.
 */
class LanguageSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum Roles : int {
        LanguageCodeRole = Qt::UserRole + 1,
    };

    explicit LanguageSortFilterProxyModel(QObject *parent = nullptr);

    /**
     * Sets the text used to filter the language list.
     *
     * The filter is applied by filterAcceptsRow() to the language code,
     * native language name, and English language name.
     */
    Q_INVOKABLE void setFilterString(const QString &filter);

    /**
     * Returns the row of a language in the proxy model.
     *
     * The returned row can be passed directly to item views using this model.
     * Returns -1 if the language is not found.
     */
    Q_INVOKABLE int rowForLanguage(const QString &language) const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};
