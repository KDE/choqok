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

#include "behaviorconfig_shorten.h"
#include <pluginmanager.h>
#include <kplugininfo.h>
#include <KDebug>
#include <KAboutApplicationDialog>
#include <qlayout.h>
#include <shortenmanager.h>
#include "choqokbehaviorsettings.h"
#include <KTabWidget>
#include <kcmoduleinfo.h>
#include <KCModuleProxy>

BehaviorConfig_Shorten::BehaviorConfig_Shorten( QWidget *parent )
    :QWidget(parent),currentShortener(0)
{
    kDebug();
    setupUi(this);
    Choqok::ShortenManager::self();
    connect(shortenPlugins, SIGNAL(currentIndexChanged(int)), SLOT(currentPluginChanged(int)));
    aboutPlugin->setIcon(KIcon("help-about"));
    configPlugin->setIcon(KIcon("configure"));
    connect( aboutPlugin, SIGNAL(clicked(bool)), SLOT(slotAboutClicked()) );
    connect( configPlugin, SIGNAL(clicked(bool)), SLOT(slotConfigureClicked()) );
}

BehaviorConfig_Shorten::~BehaviorConfig_Shorten()
{
    kDebug();
}

void BehaviorConfig_Shorten::currentPluginChanged( int index )
{
    if( shortenPlugins->itemData(index).toString() == prevShortener)
        emit changed(false);
    else
        emit changed(true);
    QString key = shortenPlugins->itemData(index).toString();
//     kDebug()<<key;
    if( !key.isEmpty() && key != "none" && availablePlugins.value(key).kcmServices().count() > 0 )
        configPlugin->setEnabled(true);
    else
        configPlugin->setEnabled(false);
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
    QList<KPluginInfo> plugins = Choqok::PluginManager::self()->availablePlugins("Shorteners");
    shortenPlugins->clear();
    shortenPlugins->addItem( i18nc("No shortener service", "None"), QLatin1String("none") );
    foreach(const KPluginInfo& plugin, plugins){
        shortenPlugins->addItem( KIcon(plugin.icon()), plugin.name(), plugin.pluginName());
        availablePlugins.insert(plugin.pluginName(), plugin);
    }
    prevShortener = Choqok::BehaviorSettings::shortenerPlugin();
    if(!prevShortener.isEmpty()) {
        shortenPlugins->setCurrentIndex(shortenPlugins->findData(prevShortener));
        //         currentPluginChanged(kcfg_plugins->currentIndex());
    }
}

void BehaviorConfig_Shorten::save()
{
    const QString shorten = shortenPlugins->itemData(shortenPlugins->currentIndex()).toString();
    Choqok::BehaviorSettings::setShortenerPlugin(shorten);
    if( prevShortener != shorten ) {
        kDebug()<<prevShortener<<" -> "<<shorten;
        Choqok::BehaviorSettings::self()->writeConfig();
        Choqok::ShortenManager::self()->reloadConfig();
    }
}

void BehaviorConfig_Shorten::slotAboutClicked()
{
    const QString shorten = shortenPlugins->itemData(shortenPlugins->currentIndex()).toString();
    if(shorten == "none")
        return;
    KPluginInfo info = availablePlugins.value(shorten);

    KAboutData aboutData(info.name().toUtf8(), info.name().toUtf8(), ki18n(info.name().toUtf8()), info.version().toUtf8(), ki18n(info.comment().toUtf8()), KAboutLicense::byKeyword(info.license()).key(), ki18n(QByteArray()), ki18n(QByteArray()), info.website().toLatin1());
    aboutData.setProgramIconName(info.icon());
    aboutData.addAuthor(ki18n(info.author().toUtf8()), ki18n(QByteArray()), info.email().toUtf8(), 0);

    KAboutApplicationDialog aboutPlugin(&aboutData, this);
    aboutPlugin.exec();
}

void BehaviorConfig_Shorten::slotConfigureClicked()
{
    kDebug();
    KPluginInfo pluginInfo = availablePlugins.value( shortenPlugins->itemData(shortenPlugins->currentIndex() ).toString() );
    kDebug()<<pluginInfo.name()<<pluginInfo.kcmServices().count();

    QPointer<KDialog> configDialog = new KDialog(this);
    configDialog->setWindowTitle(pluginInfo.name());
    // The number of KCModuleProxies in use determines whether to use a tabwidget
    KTabWidget *newTabWidget = 0;
    // Widget to use for the setting dialog's main widget,
    // either a KTabWidget or a KCModuleProxy
    QWidget * mainWidget = 0;
    // Widget to use as the KCModuleProxy's parent.
    // The first proxy is owned by the dialog itself
    QWidget *moduleProxyParentWidget = configDialog;

    foreach (const KService::Ptr &servicePtr, pluginInfo.kcmServices()) {
        if(!servicePtr->noDisplay()) {
            KCModuleInfo moduleInfo(servicePtr);
            KCModuleProxy *currentModuleProxy = new KCModuleProxy(moduleInfo, moduleProxyParentWidget);
            if (currentModuleProxy->realModule()) {
                moduleProxyList << currentModuleProxy;
                if (mainWidget && !newTabWidget) {
                    // we already created one KCModuleProxy, so we need a tab widget.
                    // Move the first proxy into the tab widget and ensure this and subsequent
                    // proxies are in the tab widget
                    newTabWidget = new KTabWidget(configDialog);
                    moduleProxyParentWidget = newTabWidget;
                    mainWidget->setParent( newTabWidget );
                    KCModuleProxy *moduleProxy = qobject_cast<KCModuleProxy*>(mainWidget);
                    if (moduleProxy) {
                        newTabWidget->addTab(mainWidget, moduleProxy->moduleInfo().moduleName());
                        mainWidget = newTabWidget;
                    } else {
                        delete newTabWidget;
                        newTabWidget = 0;
                        moduleProxyParentWidget = configDialog;
                        mainWidget->setParent(0);
                    }
                }

                if (newTabWidget) {
                    newTabWidget->addTab(currentModuleProxy, servicePtr->name());
                } else {
                    mainWidget = currentModuleProxy;
                }
            } else {
                delete currentModuleProxy;
            }
        }
    }

    // it could happen that we had services to show, but none of them were real modules.
    if (moduleProxyList.count()) {
        configDialog->setButtons(KDialog::Ok | KDialog::Cancel);

        QWidget *showWidget = new QWidget(configDialog);
        QVBoxLayout *layout = new QVBoxLayout;
        showWidget->setLayout(layout);
        layout->addWidget(mainWidget);
        layout->insertSpacing(-1, KDialog::marginHint());
        configDialog->setMainWidget(showWidget);

//         connect(&configDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));

        if (configDialog->exec() == QDialog::Accepted) {
            foreach (KCModuleProxy *moduleProxy, moduleProxyList) {
                QStringList parentComponents = moduleProxy->moduleInfo().service()->property("X-KDE-ParentComponents").toStringList();
                moduleProxy->save();
//                 foreach (const QString &parentComponent, parentComponents) {
//                     emit configCommitted(parentComponent.toLatin1());
//                 }
            }
        } else {
            foreach (KCModuleProxy *moduleProxy, moduleProxyList) {
                moduleProxy->load();
            }
        }

        qDeleteAll(moduleProxyList);
        moduleProxyList.clear();
    }
}

#include "behaviorconfig_shorten.moc"
