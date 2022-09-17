/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "shortenmanager.h"

#include <QApplication>

#include <KSharedConfig>

#include "choqokbehaviorsettings.h"
#include "libchoqokdebug.h"
#include "pluginmanager.h"
#include "notifymanager.h"

namespace Choqok
{

class ShortenManagerPrivate
{
public:
    Shortener *backend;
    ShortenManager instance;
    QRegExp findUrlRegExp;
    QRegExp removeUrlRegExp;

    ShortenManagerPrivate()
        : backend(nullptr)
    {
        findUrlRegExp.setPattern(QLatin1String("(ftps?|https?)://"));
        removeUrlRegExp.setPattern(QLatin1String("^(https?)://"));
        reloadConfig();
    }
    void reloadConfig()
    {
        const QString pluginId = Choqok::BehaviorSettings::shortenerPlugin();
        if (backend) {
            if (backend->pluginId() == pluginId) {
                return;//Already loaded
            } else {
                qCDebug(CHOQOK) << backend->pluginId();
                PluginManager::self()->unloadPlugin(backend->pluginId());
                backend = nullptr;
            }
        }
        if (pluginId.isEmpty()) {
            return;
        }
        Plugin *plugin = PluginManager::self()->loadPlugin(pluginId);
        backend = qobject_cast<Shortener *>(plugin);
        if (!backend) {
            qCDebug(CHOQOK) << "Could not load a Shortener plugin. Shortening Disabled";
        }
    }
};

Q_GLOBAL_STATIC(ShortenManagerPrivate, _smp)

QString shorten(const QString &url)
{
    return _smp->backend->shorten(url);
}

ShortenManager::ShortenManager(QObject *parent)
    : QObject(parent)
{
}

ShortenManager::~ShortenManager()
{}

ShortenManager *ShortenManager::self()
{
    return &_smp->instance;
}

QString ShortenManager::shortenUrl(const QString &url)
{
    if (_smp->backend) {
        qCDebug(CHOQOK) << "Shortening:" << url;
        NotifyManager::shortening(url);
        QString shortUrl = shorten(url);
        if (BehaviorSettings::removeHttp() && url != shortUrl) {
            shortUrl.remove(_smp->removeUrlRegExp);
        }
        return shortUrl;
    } else {
        qCDebug(CHOQOK) << "There isn't any Shortener plugin.";
        return url;
    }
}

void ShortenManager::reloadConfig()
{
    _smp->reloadConfig();
}

QString ShortenManager::parseText(const QString &text)
{
    qCDebug(CHOQOK);
    QString t;
    int i = 0, j = 0;
    while ((j = text.indexOf(_smp->findUrlRegExp, i)) != -1) {
        t += text.mid(i, j - i);
        int k = text.indexOf(QLatin1Char(' '), j);
        if (k == -1) {
            k = text.length();
        }
        QString baseUrl = text.mid(j, k - j);
        if (baseUrl.count() > 30) {
            QString tmp = Choqok::ShortenManager::self()->shortenUrl(baseUrl);
            if (BehaviorSettings::removeHttp() && tmp != baseUrl) {
                tmp.remove(_smp->removeUrlRegExp);
            }
            t += tmp;
        } else {
            t += baseUrl;
        }
        i = k;
    }
    t += text.mid(i);
    return t;
}

void ShortenManager::emitNewUnshortenedUrl(Choqok::UI::PostWidget *widget, const QUrl &fromUrl, const QUrl &toUrl)
{
    Q_EMIT newUnshortenedUrl(widget, fromUrl, toUrl);
}

}

