/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include <kglobal.h>
#include "pluginmanager.h"
#include <ksharedptr.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <KDebug>
#include "notifymanager.h"
#include <choqokbehaviorsettings.h>

namespace Choqok{

class ShortenManagerPrivate
{
public:
    Shortener *backend;
    ShortenManager instance;
    ShortenManagerPrivate()
    :backend(0)
    {
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
        return _smp->backend->shorten(url);
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
    QRegExp urlRegExp( "((ftps?|https?)://)" );
    while (( j = text.indexOf( urlRegExp, i ) ) != -1 ) {
        t += text.mid( i, j - i );
        int k = text.indexOf( ' ', j );
        if ( k == -1 )
            k = text.length();
        QString baseUrl = text.mid( j, k - j );
        if ( baseUrl.count() > 30 ) {
            t += Choqok::ShortenManager::self()->shortenUrl(baseUrl);
        } else {
            t += baseUrl;
        }
        i = k;
    }
    t += text.mid( i );
    return t;
}

}

#include "shortenmanager.moc"
