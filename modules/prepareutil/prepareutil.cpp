// SPDX-FileCopyrightText: 2023 by Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "prepareutil.h"

#include <KScreen/ConfigMonitor>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QProcess>

using namespace Qt::StringLiterals;

PrepareUtil::PrepareUtil(QObject *parent)
    : QObject{parent}
    , m_colorsSettings{new ColorsSettings(this)}
{
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished, this, [this](auto *op) {
        m_config = qobject_cast<KScreen::GetConfigOperation *>(op)->config();

        if (!m_config) {
            return;
        }

        const KScreen::OutputPtr primaryOutput = m_config->primaryOutput();
        if (primaryOutput) {
            m_scaling = primaryOutput->scale() * 100;
        } else {
            m_scaling = 100;
        }

        Q_EMIT scalingChanged();
    });

    // set property initially
    m_usingDarkTheme = m_colorsSettings->colorScheme() == u"BreezeDark"_s;
}

int PrepareUtil::scaling() const
{
    return m_scaling;
}

void PrepareUtil::setScaling(int scaling)
{
    if (!m_config) {
        return;
    }

    // TODO: Find the display the app is on and only set the scaling for that display instead.
    // Setting for all displays is not ideal since they can have different densities.
    const auto output = m_config->primaryOutput();
    qreal scalingNum = ((double)scaling) / 100;
    if (!output) {
        return;
    }

    output->setScale(scalingNum);

    auto setop = new KScreen::SetConfigOperation(m_config, this);
    setop->exec();

    m_scaling = scaling;
    Q_EMIT scalingChanged();
}

QStringList PrepareUtil::scalingOptions()
{
    return {
        u"50%"_s,
        u"75%"_s,
        u"100%"_s,
        u"125%"_s,
        u"150%"_s,
        u"175%"_s,
        u"200%"_s,
        u"225%"_s,
        u"250%"_s,
        u"275%"_s,
        u"300%"_s,
    };
}

bool PrepareUtil::usingDarkTheme() const
{
    return m_usingDarkTheme;
}

void PrepareUtil::setUsingDarkTheme(bool usingDarkTheme)
{
    // use plasma-apply-lookandfeel since it has logic for notifying the shell of changes
    if (usingDarkTheme) {
        QProcess::execute(u"plasma-apply-lookandfeel"_s, {QStringLiteral("--apply"), QStringLiteral("org.kde.breezedark.desktop")});
    } else {
        QProcess::execute(u"plasma-apply-lookandfeel"_s, {QStringLiteral("--apply"), QStringLiteral("org.kde.breeze.desktop")});
    }

    m_usingDarkTheme = usingDarkTheme;
    Q_EMIT usingDarkThemeChanged();
}

#include "moc_prepareutil.cpp"
