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

#include "is_gd_config.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>

#include "notifymanager.h"

#include "is_gd_settings.h"

K_PLUGIN_FACTORY_WITH_JSON( Is_gd_ConfigFactory, "choqok_is_gd_config.json",
                            registerPlugin < Is_gd_Config > (); )

Is_gd_Config::Is_gd_Config(QWidget* parent, const QVariantList& ):
        KCModule( KAboutData::pluginData("kcm_choqok_is_gd"), parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mIsGdCtl");
    wd->setMinimumWidth(200);
    ui.setupUi(wd);
    addConfig( Is_gd_Settings::self(), wd );
    layout->addWidget(wd);
}

Is_gd_Config::~Is_gd_Config()
{
}

void Is_gd_Config::load()
{
    KCModule::load();
}

void Is_gd_Config::save()
{
    KCModule::save();
}

void Is_gd_Config::emitChanged()
{
    Q_EMIT changed( true );
}

#include "is_gd_config.moc"
