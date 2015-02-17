/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include <QVBoxLayout>

#include <KPluginFactory>
#include <KLocale>
#include <KMessageBox>
#include "choqokdebug.h"

#include "accountmanager.h"
#include "passwordmanager.h"

#include "twitpicsettings.h"

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
    connect(ui.accountsList, SIGNAL(currentIndexChanged(int)), SLOT(emitChanged()));
}

TwitpicConfig::~TwitpicConfig()
{
    qCDebug(CHOQOK)<<TwitpicSettings::alias();
}

void TwitpicConfig::load()
{
    qCDebug(CHOQOK);
    KCModule::load();
    QList<Choqok::Account*> list = Choqok::AccountManager::self()->accounts();
    Q_FOREACH (Choqok::Account *acc, list) {
        if(acc->inherits("TwitterAccount")){
            ui.accountsList->addItem(acc->alias());
        }
    }
    TwitpicSettings::self()->readConfig();
    ui.accountsList->setCurrentItem(TwitpicSettings::alias());
}

void TwitpicConfig::save()
{
    qCDebug(CHOQOK)<<ui.accountsList->currentIndex();
    if(ui.accountsList->currentIndex() > -1) {
        TwitpicSettings::setAlias( ui.accountsList->currentText() );
        qCDebug(CHOQOK)<<TwitpicSettings::alias();
    } else {
        TwitpicSettings::setAlias(QString());
        KMessageBox::error(this, i18n("You have to configure at least one Twitter account to use this plugin."));
    }
    TwitpicSettings::self()->writeConfig();
    KCModule::save();
}

void TwitpicConfig::emitChanged()
{
    Q_EMIT changed(true);
}

#include "twitpicconfig.moc"
