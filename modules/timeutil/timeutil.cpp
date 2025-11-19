// SPDX-FileCopyrightText: 2023 by Devin Lin <devin@kde.org>
// SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "timeutil.h"

#include "timedate_interface.h"

#include <QDBusPendingCallWatcher>
#include <QTimeZone>

using namespace Qt::StringLiterals;

TimeUtil::TimeUtil(QObject *parent)
    : QObject{parent}
    , m_dbusInterface(new OrgFreedesktopTimedate1Interface(u"org.freedesktop.timedate1"_s, u"/org/freedesktop/timedate1"_s, QDBusConnection::systemBus(), this))
{
}

QString TimeUtil::currentTimeZone() const
{
    return QString::fromUtf8(QTimeZone::systemTimeZoneId());
}

void TimeUtil::setCurrentTimeZone(const QString &timeZone)
{
    auto reply = m_dbusInterface->SetTimezone(timeZone, false);
    auto watcher = new QDBusPendingCallWatcher(reply, nullptr);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, reply]() {
        if (reply.isValid()) {
            Q_EMIT currentTimeZoneChanged();
        }
    });
}

#include "moc_timeutil.cpp"
