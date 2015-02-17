/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "shortenmanager.h"

#include <QApplication>
#include <QtConcurrentRun>

#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KSharedConfig>
#include <KSharedPtr>

#include "choqokbehaviorsettings.h"
#include "pluginmanager.h"
#include "notifymanager.h"

namespace Choqok{

class ShortenManagerPrivate
{
public:
    Shortener *backend;
    ShortenManager instance;
    QRegExp findUrlRegExp;
    QRegExp removeUrlRegExp;

    ShortenManagerPrivate()
    :backend(0)
    {
        findUrlRegExp.setPattern( "(ftps?|https?)://" );
        removeUrlRegExp.setPattern( "^(https?)://" );
        reloadConfig();
    }
    void reloadConfig()
    {
        const QString pluginId = Choqok::BehaviorSettings::shortenerPlugin();
        if(backend){
            if(backend->pluginName() == pluginId) {
                return;//Already loaded
            }else{
                kDebug()<<backend->pluginName();
                PluginManager::self()->unloadPlugin(backend->pluginName());
                backend = 0L;
            }
        }
        if(pluginId.isEmpty())
            return;
        Plugin *plugin = PluginManager::self()->loadPlugin(pluginId);
        backend = qobject_cast<Shortener*>( plugin );
        if(!backend){
            kDebug()<<"Could not load a Shortener plugin. Shortening Disabled";
        }
    }
};

K_GLOBAL_STATIC(ShortenManagerPrivate, _smp)

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
    if(_smp->backend){
        kDebug()<<"Shortening: "<<url;
        NotifyManager::shortening(url);
// #ifndef QT_NO_CONCURRENT
        ///Commented due KIO doesn't support running in another thread
        ///We need another solution :)
//         QFuture<QString> res = QtConcurrent::run<QString>( shorten, QString(url));
//         while( res.isRunning() )
//             QApplication::processEvents();
//         return res.result();
// #else
        QString shortUrl = shorten(url);
        if(BehaviorSettings::removeHttp() && url != shortUrl)
                shortUrl.remove(_smp->removeUrlRegExp);
        return shortUrl;
// #endif
    } else {
        kDebug()<<"There isn't any Shortener plugin.";
        return url;
    }
}

void ShortenManager::reloadConfig()
{
    _smp->reloadConfig();
}

QString ShortenManager::parseText(const QString &text)
{
    kDebug();
    QString t = "";
    int i = 0, j = 0;
    while (( j = text.indexOf( _smp->findUrlRegExp, i ) ) != -1 ) {
        t += text.mid( i, j - i );
        int k = text.indexOf( ' ', j );
        if ( k == -1 )
            k = text.length();
        QString baseUrl = text.mid( j, k - j );
        if ( baseUrl.count() > 30 ) {
            QString tmp = Choqok::ShortenManager::self()->shortenUrl(baseUrl);
            if(BehaviorSettings::removeHttp() && tmp != baseUrl)
                tmp.remove(_smp->removeUrlRegExp);
            t += tmp;
        } else {
            t += baseUrl;
        }
        i = k;
    }
    t += text.mid( i );
    return t;
}

void ShortenManager::emitNewUnshortenedUrl(Choqok::UI::PostWidget* widget, const KUrl& fromUrl, const KUrl& toUrl)
{
    Q_EMIT newUnshortenedUrl(widget, fromUrl, toUrl);
}


}

