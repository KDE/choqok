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
#include "appearanceconfig.h"
#include "ui_appearanceconfig_base.h"

#include <kdebug.h>
#include <kpluginfactory.h>
#include <klocale.h>

#include <ktabwidget.h>

//class AppearanceConfig;
#include <choqokappearancesettings.h>

K_PLUGIN_FACTORY( ChoqokAppearanceConfigFactory,
        registerPlugin<AppearanceConfig>(); )
K_EXPORT_PLUGIN( ChoqokAppearanceConfigFactory("kcm_choqok_appearanceconfig") )

class AppearanceConfig::Private
{
public:
    Private()
     : mAppearanceTabCtl(0L)
    {}

    KTabWidget *mAppearanceTabCtl;

    Ui::AppearanceConfig_Base mPrefsBase;
};


AppearanceConfig::AppearanceConfig(QWidget *parent, const QVariantList &args )
: KCModule( ChoqokAppearanceConfigFactory::componentData(), parent, args ), d(new Private())
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    // since the tab widget is already within a layout with margins in the KSettings::Dialog
    // it needs no margins of its own.
    layout->setContentsMargins( 0, 0, 0, 0 );
    d->mAppearanceTabCtl = new KTabWidget(this);
    d->mAppearanceTabCtl->setTabBarHidden(true);
    d->mAppearanceTabCtl->setDocumentMode(true);
    d->mAppearanceTabCtl->setObjectName("mAppearanceTabCtl");
    layout->addWidget( d->mAppearanceTabCtl );

//     KConfigGroup config(KGlobal::config(), "ChatWindowSettings");

    // "Contact List" TAB =======================================================
    QWidget *appearsWidget = new QWidget(d->mAppearanceTabCtl);
    d->mPrefsBase.setupUi(appearsWidget);
    addConfig( Choqok::AppearanceSettings::self(), appearsWidget );

    d->mAppearanceTabCtl->addTab(appearsWidget, i18n("Appearance"));

    // ==========================================================================

    load();
}

AppearanceConfig::~AppearanceConfig()
{
    delete d;
}

void AppearanceConfig::save()
{
    KCModule::save();
//    kDebug(14000) << "called.";
}

void AppearanceConfig::load()
{
    KCModule::load();
//    kDebug(14000) << "called";
}

#include "appearanceconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
