/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "behaviorconfig_shorten.h"
#include <pluginmanager.h>
#include <kplugininfo.h>
#include <KDebug>
#include <qlayout.h>
#include <shortenmanager.h>

BehaviorConfig_Shorten::BehaviorConfig_Shorten( QWidget *parent )
    :QWidget(parent),currentShortener(0)
{
    kDebug();
    setupUi(this);
    availablePlugins = Choqok::PluginManager::self()->availablePlugins("Shorteners");
    foreach(const KPluginInfo& plugin, availablePlugins){
        kcfg_plugins->addItem( KIcon(plugin.icon()), plugin.name(), plugin.pluginName());
    }
    prevShortener = KGlobal::config()->group("Advanced").readEntry("ShortenPlugin", QString());
    if(!prevShortener.isEmpty()) {
        kcfg_plugins->setCurrentIndex(kcfg_plugins->findData(prevShortener));
//         currentPluginChanged(kcfg_plugins->currentIndex());
    }
//     connect(kcfg_plugins, SIGNAL(currentIndexChanged(int)), SLOT(currentPluginChanged(int)));
}

BehaviorConfig_Shorten::~BehaviorConfig_Shorten()
{
    kDebug();
    const QString shorten = kcfg_plugins->itemData(kcfg_plugins->currentIndex()).toString();
    KGlobal::config()->group("Advanced").writeEntry("ShortenPlugin", shorten);
    if( prevShortener != shorten )
        Choqok::ShortenManager::self()->reloadConfig();
}

void BehaviorConfig_Shorten::currentPluginChanged( int index )
{
    kDebug();
    if(currentShortener){
        layout()->removeWidget(currentShortener->configWidget());
//         currentShortener->deleteLater();
//         currentShortener = 0;
    }
    currentShortener = qobject_cast<Choqok::Shortener*>(Choqok::PluginManager::self()->loadPlugin(
                                                            kcfg_plugins->itemData(index).toString() ) );
    if(currentShortener && currentShortener->configWidget())
        layout()->addWidget(currentShortener->configWidget());
}

