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
#include "choqokbehaviorsettings.h"

BehaviorConfig_Shorten::BehaviorConfig_Shorten( QWidget *parent )
    :QWidget(parent),currentShortener(0)
{
    kDebug();
    setupUi(this);
    Choqok::ShortenManager::self();
    connect(kcfg_shortenPlugins, SIGNAL(currentIndexChanged(int)), SLOT(currentPluginChanged(int)));
}

BehaviorConfig_Shorten::~BehaviorConfig_Shorten()
{
    kDebug();
}

void BehaviorConfig_Shorten::currentPluginChanged( int index )
{
    if( kcfg_shortenPlugins->itemData(index).toString() == prevShortener)
        emit changed(false);
    else
        emit changed(true);
//     if(currentShortener){
//         layout()->removeWidget(currentShortener->configWidget());
// //         currentShortener->deleteLater();
// //         currentShortener = 0;
//     }
//     currentShortener = qobject_cast<Choqok::Shortener*>(Choqok::PluginManager::self()->loadPlugin(
//                                                             kcfg_plugins->itemData(index).toString() ) );
//     if(currentShortener && currentShortener->configWidget())
//         layout()->addWidget(currentShortener->configWidget());
}

void BehaviorConfig_Shorten::load()
{
    availablePlugins = Choqok::PluginManager::self()->availablePlugins("Shorteners");
    kcfg_shortenPlugins->clear();
    kcfg_shortenPlugins->addItem( i18n("None") );
    foreach(const KPluginInfo& plugin, availablePlugins){
        kcfg_shortenPlugins->addItem( KIcon(plugin.icon()), plugin.name(), plugin.pluginName());
    }
    prevShortener = Choqok::BehaviorSettings::shortenerPlugin();
    if(!prevShortener.isEmpty()) {
        kcfg_shortenPlugins->setCurrentIndex(kcfg_shortenPlugins->findData(prevShortener));
        //         currentPluginChanged(kcfg_plugins->currentIndex());
    }
}

void BehaviorConfig_Shorten::save()
{
    const QString shorten = kcfg_shortenPlugins->itemData(kcfg_shortenPlugins->currentIndex()).toString();
    Choqok::BehaviorSettings::setShortenerPlugin(shorten);
    if( prevShortener != shorten ) {
        kDebug()<<prevShortener<<" -> "<<shorten;
        Choqok::BehaviorSettings::self()->writeConfig();
        Choqok::ShortenManager::self()->reloadConfig();
    }
}

