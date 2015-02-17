/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "behaviorconfig.h"

#include "ui_behaviorconfig_general.h"
#include "ui_behaviorconfig_notifications.h"

#include <QVBoxLayout>

#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KTabWidget>

#include "behaviorconfig_shorten.h"
#include "behaviordebug.h"
#include "choqokbehaviorsettings.h"

K_PLUGIN_FACTORY( ChoqokBehaviorConfigFactory,
                  registerPlugin <BehaviorConfig>(); )
K_EXPORT_PLUGIN( ChoqokBehaviorConfigFactory("kcm_choqok_behaviorconfig") )

class BehaviorConfig::Private
{
public:
    KTabWidget* mBehaviorTabCtl;

    Ui_BehaviorConfig_General mPrfsGeneral;
    Ui_BehaviorConfig_Notifications mPrfsNotify;
    BehaviorConfig_Shorten *mPrfsShorten;
    KCModuleProxy *proxyModule;
};

BehaviorConfig::BehaviorConfig(QWidget *parent, const QVariantList &args) :
        KCModule( ChoqokBehaviorConfigFactory::componentData(), parent, args ), d(new Private)
{
    qCDebug(CHOQOK);
    QVBoxLayout *layout = new QVBoxLayout(this);
    // since KSetting::Dialog has margins here, we don't need our own.
    layout->setContentsMargins( 0, 0, 0, 0);

    d->mBehaviorTabCtl = new KTabWidget(this);
    d->mBehaviorTabCtl->setObjectName("mBehaviorTabCtl");
    layout->addWidget( d->mBehaviorTabCtl );

    // "General" TAB ============================================================
    QWidget *mPrfsGeneralDlg = new QWidget(d->mBehaviorTabCtl);
    d->mPrfsGeneral.setupUi(mPrfsGeneralDlg);
    addConfig( Choqok::BehaviorSettings::self(), mPrfsGeneralDlg );
    d->mBehaviorTabCtl->addTab(mPrfsGeneralDlg, i18n("&General"));
    d->mPrfsGeneral.kcfg_updateInterval->setSuffix(ki18np(" Minute", " Minutes"));

#ifdef QTINDICATE_BUILD
    // "Notifications" TAB ============================================================
    QWidget *mPrfsNotifyDlg = new QWidget(d->mBehaviorTabCtl);
    d->mPrfsNotify.setupUi(mPrfsNotifyDlg);
    addConfig( Choqok::BehaviorSettings::self(), mPrfsNotifyDlg);
    d->mBehaviorTabCtl->addTab(mPrfsNotifyDlg, i18n("&Notifications"));
    d->mPrfsNotify.kcfg_notifyInterval->setSuffix(ki18np(" Second", " Seconds"));
    /* Remove below code, when all functions on tab will work*/
    d->mPrfsNotify.kcfg_notifyInterval->setVisible(false);
    d->mPrfsNotify.kcfg_showAllNotifiesInOne->setVisible(false);
    d->mPrfsNotify.label_4->setVisible(false);
    /*     */
#endif

    // "Shortening" TAB ===============================================================
    d->mPrfsShorten = new BehaviorConfig_Shorten(d->mBehaviorTabCtl);
    addConfig( Choqok::BehaviorSettings::self(), d->mPrfsShorten );
    d->mBehaviorTabCtl->addTab(d->mPrfsShorten, i18n("URL &Shortening"));

    KCModuleInfo proxyInfo("proxy.desktop");
    d->proxyModule = new KCModuleProxy(proxyInfo,parent);
    d->mBehaviorTabCtl->addTab( d->proxyModule, i18n(proxyInfo.moduleName().toUtf8()) );

    connect(d->mPrfsShorten, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)) );
    connect(d->proxyModule,   SIGNAL( changed(bool) ), this, SIGNAL( changed(bool) ) );

    load();

}

BehaviorConfig::~BehaviorConfig()
{
    delete d;
}

void BehaviorConfig::save()
{
   qCDebug(CHOQOK);

    KCModule::save();
    d->mPrfsShorten->save();
    d->proxyModule->save();
//     Choqok::BehaviorSettings::self()->writeConfig();

    load();
}

void BehaviorConfig::load()
{
    KCModule::load();
    d->mPrfsShorten->load();
    d->proxyModule->load();
}

#include "behaviorconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
