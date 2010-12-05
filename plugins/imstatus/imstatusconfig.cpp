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

#include "imstatusconfig.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include "imstatussettings.h"
#include <QVBoxLayout>
#include <KDebug>
#include "imqdbus.h"

K_PLUGIN_FACTORY ( IMStatusConfigFactory, registerPlugin < IMStatusConfig > (); )
K_EXPORT_PLUGIN ( IMStatusConfigFactory ( "kcm_choqok_imstatus" ) )

IMStatusConfig::IMStatusConfig ( QWidget* parent, const QVariantList& args ) :
        KCModule ( IMStatusConfigFactory::componentData(), parent, args )
{
    QVBoxLayout *layout = new QVBoxLayout ( this );
    QWidget *wd = new QWidget ( this );
    wd->setObjectName ( "mIMStatusCtl" );
    ui.setupUi ( wd );
    addConfig ( IMStatusSettings::self(), wd );
    layout->addWidget ( wd );
    setButtons ( KCModule::Apply );
    connect ( ui.cfg_imclient, SIGNAL ( currentIndexChanged ( int ) ), SLOT ( emitChanged() ) );
    connect ( ui.cfg_repeat, SIGNAL ( stateChanged ( int ) ), SLOT ( emitChanged() ) );
    connect ( ui.cfg_reply, SIGNAL ( stateChanged ( int ) ), SLOT ( emitChanged() ) );
    connect ( ui.cfg_templtate, SIGNAL ( textChanged() ), SLOT ( emitChanged() ) );
    imList = IMQDBus::scanForIMs();
    ui.cfg_imclient->addItems ( imList );
}

IMStatusConfig::~IMStatusConfig()
{

}

void IMStatusConfig::load()
{
    kDebug();
    KCModule::load();
    KConfigGroup grp ( KGlobal::config(), "IMStatus" );
    IMStatusSettings::self()->readConfig();
    ui.cfg_imclient->setCurrentIndex ( imList.indexOf ( IMStatusSettings::imclient() ) );
    ui.cfg_templtate->setPlainText ( IMStatusSettings::templtate().isEmpty() ?
                                     "%username%: \"%status%\" at %time% from %client% (%url%)" : IMStatusSettings::templtate() );
    ui.cfg_reply->setChecked ( IMStatusSettings::reply() );
    ui.cfg_repeat->setChecked ( IMStatusSettings::repeat() );
}

void IMStatusConfig::save()
{
    kDebug();
    KCModule::save();
    IMStatusSettings::setImclient ( ui.cfg_imclient->currentText() );
    IMStatusSettings::setTempltate ( ui.cfg_templtate->toPlainText() );
    IMStatusSettings::setReply ( ui.cfg_reply->isChecked() );
    IMStatusSettings::setRepeat ( ui.cfg_repeat->isChecked() );
    IMStatusSettings::self()->writeConfig();
}

void IMStatusConfig::emitChanged()
{
    emit changed ( true );
}

#include "imstatusconfig.moc"
