/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Andrey Esin <gmlastik@gmail.com>

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

#include "tinyarro_ws_config.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include "tinyarro_ws_settings.h"
#include <QVBoxLayout>
#include <passwordmanager.h>

K_PLUGIN_FACTORY( Tinyarro_ws_ConfigFactory, registerPlugin < Tinyarro_ws_Config > (); )
K_EXPORT_PLUGIN( Tinyarro_ws_ConfigFactory( "kcm_choqok_tinyarro_ws" ) )

Tinyarro_ws_Config::Tinyarro_ws_Config(QWidget* parent, const QVariantList& ):
        KCModule( Tinyarro_ws_ConfigFactory::componentData(), parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    QWidget *wd = new QWidget( this );
    wd->setObjectName( "mTinyarro_ws_Ctl" );
    ui.setupUi(wd);
    addConfig( Tinyarro_ws_Settings::self(), wd );
    layout->addWidget( wd );
    QString domain = ".ws";
    hostList.insert( QChar( 0x27A8 ) + domain, "xn--ogi.ws" );
    hostList.insert( QChar( 0x27AF ) + domain, "xn--vgi.ws" );
    hostList.insert( QChar( 0x2794 ) + domain, "xn--3fi.ws" );
    hostList.insert( QChar( 0x279E ) + domain, "xn--egi.ws" );
    hostList.insert( QChar( 0x27BD ) + domain, "xn--9gi.ws" );
    hostList.insert( QChar( 0x27B9 ) + domain, "xn--5gi.ws" );
    hostList.insert( QChar( 0x2729 ) + domain, "xn--1ci.ws" );
    hostList.insert( QChar( 0x273F ) + domain, "xn--odi.ws" );
    hostList.insert( QChar( 0x2765 ) + domain, "xn--rei.ws" );
    hostList.insert( QChar( 0x203A ) + domain, "xn--cwg.ws" );
    hostList.insert( QChar( 0x2318 ) + domain, "xn--bih.ws" );
    hostList.insert( QChar( 0x203D ) + domain, "xn--fwg.ws" );
    hostList.insert( QChar( 0x2601 ) + domain, "xn--l3h.ws" );
    hostList.insert( "ta.gd",                  "ta.gd" );
    hostList.insert( i18n( "Random host" ),    "Random" );

    QMap<QString, QString>::const_iterator i = hostList.constBegin();
    while ( i != hostList.constEnd() ) {
     ui.kcfg_tinyarro_ws_host->addItem( i.key() );
     ++i;
    }

    connect( ui.kcfg_tinyarro_ws_host,SIGNAL(currentIndexChanged(int)), SLOT(emitChanged()) );
}

Tinyarro_ws_Config::~Tinyarro_ws_Config()
{
}

void Tinyarro_ws_Config::load()
{
    kDebug();
    KCModule::load();
    KConfigGroup grp(KGlobal::config(), "Tinyarro.ws Shortener");
    ui.kcfg_tinyarro_ws_host->setCurrentIndex(grp.readEntry("tinyarro_ws_host", "0").toInt());
}

void Tinyarro_ws_Config::save()
{
    kDebug();
    KCModule::save();
    KConfigGroup grp(KGlobal::config(), "Tinyarro.ws Shortener");
    grp.writeEntry("tinyarro_ws_host", ui.kcfg_tinyarro_ws_host->currentIndex());
    grp.writeEntry("tinyarro_ws_host_punny", hostList[ui.kcfg_tinyarro_ws_host->currentText()]);
}

void Tinyarro_ws_Config::emitChanged()
{
    emit changed(true);
}

#include "tinyarro_ws_config.moc"
