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

#include "twitpicconfig.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include "twitpicsettings.h"
#include <QVBoxLayout>
#include <passwordmanager.h>

K_PLUGIN_FACTORY( TwitpicConfigFactory, registerPlugin < TwitpicConfig > (); )
K_EXPORT_PLUGIN( TwitpicConfigFactory( "kcm_choqok_twitpic" ) )

TwitpicConfig::TwitpicConfig(QWidget* parent, const QVariantList& ):
        KCModule( TwitpicConfigFactory::componentData(), parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mTwitpicCtl");
    ui.setupUi(wd);
    addConfig( TwitpicSettings::self(), wd );
    layout->addWidget(wd);
    connect( ui.kcfg_username,SIGNAL(textChanged(QString)), SLOT(emitChanged()) );
    connect( ui.cfg_password, SIGNAL(textChanged(QString)), SLOT(emitChanged()) );
}

TwitpicConfig::~TwitpicConfig()
{

}

void TwitpicConfig::load()
{
    kDebug();
    KCModule::load();
    ui.cfg_password->setText( Choqok::PasswordManager::self()->readPassword( QString("twitpic_%1")
                                                                      .arg(ui.kcfg_username->text()) ) );
}

void TwitpicConfig::save()
{
    kDebug();
    KCModule::save();
    Choqok::PasswordManager::self()->writePassword(QString("twitpic_%1").arg(ui.kcfg_username->text()),
                                                   ui.cfg_password->text());
}

void TwitpicConfig::emitChanged()
{
    emit changed(true);
    disconnect( ui.kcfg_username, SIGNAL(textChanged(QString)), this, SLOT(emitChanged()) );
    disconnect( ui.cfg_password, SIGNAL(textChanged(QString)), this, SLOT(emitChanged()) );
}

#include "twitpicconfig.moc"
