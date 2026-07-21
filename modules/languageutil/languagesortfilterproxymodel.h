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
    explicit LanguageSortFilterProxyModel(QObject *parent = nullptr);

    void setFilterString(const QString &filter);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QString m_filterString;
};
