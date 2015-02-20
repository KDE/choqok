/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#include "mobypictureconfig.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>

#include "mobypicturesettings.h"

#include "accountmanager.h"
#include "passwordmanager.h"

K_PLUGIN_FACTORY_WITH_JSON( MobypictureConfigFactory, "choqok_mobypicture_config.json",
                            registerPlugin < MobypictureConfig > (); )
K_EXPORT_PLUGIN ( MobypictureConfigFactory ( "kcm_choqok_mobypicture" ) )

MobypictureConfig::MobypictureConfig ( QWidget* parent, const QVariantList& )
    : KCModule ( KAboutData::pluginData("kcm_choqok_mobypicture"), parent )
{
    QVBoxLayout *layout = new QVBoxLayout ( this );
    QWidget *wd = new QWidget ( this );
    wd->setObjectName ( "mMobypictureCtl" );
    ui.setupUi ( wd );
    addConfig ( MobypictureSettings::self(), wd );
    layout->addWidget ( wd );
    ui.cfg_pin->setEchoMode( QLineEdit::Password );
    connect ( ui.cfg_basic, SIGNAL ( clicked(bool) ), SLOT ( emitChanged() ) );
    connect ( ui.cfg_login, SIGNAL ( textChanged(QString)), SLOT ( emitChanged() ) );
    connect ( ui.cfg_pin, SIGNAL ( textChanged(QString)), SLOT ( emitChanged() ) );   
    connect ( ui.cfg_oauth, SIGNAL ( clicked(bool)), SLOT ( emitChanged() ) );
    connect ( ui.cfg_accountsList, SIGNAL ( currentIndexChanged ( int ) ), SLOT ( emitChanged() ) );
}

MobypictureConfig::~MobypictureConfig()
{
}

void MobypictureConfig::load()
{
    KCModule::load();
    QList<Choqok::Account*> list = Choqok::AccountManager::self()->accounts();
    Q_FOREACH (Choqok::Account *acc, list) {
        if ( acc->inherits ( "TwitterAccount" ) ) {
            ui.cfg_accountsList->addItem ( acc->alias() );
        }
    }
    MobypictureSettings::self()->readConfig();
    ui.cfg_basic->setChecked(MobypictureSettings::basic());
    ui.cfg_login->setText(MobypictureSettings::login());
    ui.cfg_pin->setText( Choqok::PasswordManager::self()->readPassword ( QString ( "mobypicture_%1" )
                          .arg ( ui.cfg_login->text() ) ));
    ui.cfg_oauth->setChecked(MobypictureSettings::oauth());
    ui.cfg_accountsList->setCurrentItem ( MobypictureSettings::alias() );
    emitChanged();
}

void MobypictureConfig::save()
{
    if ( ui.cfg_accountsList->currentIndex() > -1 ) {
        MobypictureSettings::setAlias ( ui.cfg_accountsList->currentText() );
    } else {
        MobypictureSettings::setAlias ( QString() );
        KMessageBox::error ( this, i18n ( "You have to configure at least one Twitter account to use this plugin." ) );
    }
    
    MobypictureSettings::setBasic(ui.cfg_basic->isChecked());
    MobypictureSettings::setLogin(ui.cfg_login->text());
    Choqok::PasswordManager::self()->writePassword(QString("mobypicture_%1").arg(ui.cfg_login->text()),
                                                   ui.cfg_pin->text());
    MobypictureSettings::setOauth(ui.cfg_oauth->isChecked());
    MobypictureSettings::self()->writeConfig();
    KCModule::save();
}

void MobypictureConfig::emitChanged()
{
    ui.cfg_login->setEnabled(ui.cfg_basic->isChecked());
    ui.cfg_pin->setEnabled(ui.cfg_basic->isChecked());
    ui.cfg_accountsList->setEnabled(ui.cfg_oauth->isChecked());
    Q_EMIT changed ( true );
}

#include "mobypictureconfig.moc"
