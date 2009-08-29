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

#include "nowlisteningconfig.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include <nowlisteningsettings.h>
#include <QVBoxLayout>

K_PLUGIN_FACTORY( NowListeningConfigFactory, registerPlugin < NowListeningConfig > (); )
K_EXPORT_PLUGIN( NowListeningConfigFactory( "kcm_choqok_nowlistening" ) )

NowListeningConfig::NowListeningConfig(QWidget* parent, const QVariantList& args):
        KCModule( NowListeningConfigFactory::componentData(), parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    ui.setupUi(wd);
    addConfig( NowListeningSettings::self(), wd);
    layout->addWidget(wd);
}

NowListeningConfig::~NowListeningConfig()
{

}

void NowListeningConfig::defaults()
{
    KCModule::defaults();
}

void NowListeningConfig::load()
{
    KCModule::load();
}

void NowListeningConfig::save()
{
    KCModule::save();
}

#include "nowlisteningconfig.moc"
