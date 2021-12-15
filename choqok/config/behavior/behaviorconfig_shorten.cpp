/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "behaviorconfig_shorten.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QTabWidget>

#include <KAboutData>
#include <KAboutApplicationDialog>
#include <KCModuleProxy>
#include <KPluginInfo>

#include "choqokbehaviorsettings.h"
#include "behaviordebug.h"
#include "pluginmanager.h"
#include "shortenmanager.h"

BehaviorConfig_Shorten::BehaviorConfig_Shorten(QWidget *parent)
    : QWidget(parent), currentShortener(nullptr)
{
    qCDebug(CHOQOK);
    setupUi(this);
    Choqok::ShortenManager::self();
    connect(shortenPlugins, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &BehaviorConfig_Shorten::currentPluginChanged);
    connect(aboutPlugin,  &QPushButton::clicked, this, &BehaviorConfig_Shorten::slotAboutClicked);
    connect(configPlugin, &QPushButton::clicked, this, &BehaviorConfig_Shorten::slotConfigureClicked);
}

BehaviorConfig_Shorten::~BehaviorConfig_Shorten()
{
    qCDebug(CHOQOK);
}

void BehaviorConfig_Shorten::currentPluginChanged(int index)
{
    if (shortenPlugins->itemData(index).toString() == prevShortener) {
        Q_EMIT changed(false);
    } else {
        Q_EMIT changed(true);
    }
    QString key = shortenPlugins->itemData(index).toString();
//     qCDebug(CHOQOK)<<key;
    if (!key.isEmpty() && key != QLatin1String("none") && availablePlugins.value(key).kcmServices().count() > 0) {
        configPlugin->setEnabled(true);
    } else {
        configPlugin->setEnabled(false);
    }
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
    QList<KPluginInfo> plugins = Choqok::PluginManager::self()->availablePlugins(QLatin1String("Shorteners"));
    shortenPlugins->clear();
    shortenPlugins->addItem(i18nc("No shortener service", "None"), QLatin1String("none"));
    for (const KPluginInfo &plugin: plugins) {
        shortenPlugins->addItem(QIcon::fromTheme(plugin.icon()), plugin.name(), plugin.pluginName());
        availablePlugins.insert(plugin.pluginName(), plugin);
    }
    prevShortener = Choqok::BehaviorSettings::shortenerPlugin();
    if (!prevShortener.isEmpty()) {
        shortenPlugins->setCurrentIndex(shortenPlugins->findData(prevShortener));
        //         currentPluginChanged(kcfg_plugins->currentIndex());
    }
}

void BehaviorConfig_Shorten::save()
{
    const QString shorten = shortenPlugins->itemData(shortenPlugins->currentIndex()).toString();
    Choqok::BehaviorSettings::setShortenerPlugin(shorten);
    if (prevShortener != shorten) {
        qCDebug(CHOQOK) << prevShortener << "->" << shorten;
        Choqok::BehaviorSettings::self()->save();
        Choqok::ShortenManager::self()->reloadConfig();
    }
}

void BehaviorConfig_Shorten::slotAboutClicked()
{
    const QString shorten = shortenPlugins->itemData(shortenPlugins->currentIndex()).toString();
    if (shorten == QLatin1String("none")) {
        return;
    }
    KPluginInfo info = availablePlugins.value(shorten);

    KAboutData aboutData(info.name(), info.name(), info.version(), info.comment(),
                         KAboutLicense::byKeyword(info.license()).key(), QString(),
                         QString(), info.website());
    aboutData.addAuthor(info.author(), QString(), info.email());

    KAboutApplicationDialog aboutPlugin(aboutData, this);
    aboutPlugin.setWindowIcon(QIcon::fromTheme(info.icon()));
    aboutPlugin.exec();
}

void BehaviorConfig_Shorten::slotConfigureClicked()
{
    qCDebug(CHOQOK);
    KPluginInfo pluginInfo = availablePlugins.value(shortenPlugins->itemData(shortenPlugins->currentIndex()).toString());
    qCDebug(CHOQOK) << pluginInfo.name() << pluginInfo.kcmServices().count();

    QPointer<QDialog> configDialog = new QDialog(this);
    configDialog->setWindowTitle(pluginInfo.name());
    // The number of KCModuleProxies in use determines whether to use a tabwidget
    QTabWidget *newTabWidget = nullptr;
    // Widget to use for the setting dialog's main widget,
    // either a QTabWidget or a KCModuleProxy
    QWidget *mainWidget = nullptr;
    // Widget to use as the KCModuleProxy's parent.
    // The first proxy is owned by the dialog itself
    QWidget *moduleProxyParentWidget = configDialog;

    for (const KService::Ptr &servicePtr: pluginInfo.kcmServices()) {
        if (!servicePtr->noDisplay()) {
            KCModuleProxy *currentModuleProxy = new KCModuleProxy(servicePtr, moduleProxyParentWidget);
            if (currentModuleProxy->realModule()) {
                moduleProxyList << currentModuleProxy;
                if (mainWidget && !newTabWidget) {
                    // we already created one KCModuleProxy, so we need a tab widget.
                    // Move the first proxy into the tab widget and ensure this and subsequent
                    // proxies are in the tab widget
                    newTabWidget = new QTabWidget(configDialog);
                    moduleProxyParentWidget = newTabWidget;
                    mainWidget->setParent(newTabWidget);
                    KCModuleProxy *moduleProxy = qobject_cast<KCModuleProxy *>(mainWidget);
                    if (moduleProxy) {
                        newTabWidget->addTab(mainWidget, servicePtr->name());
                        mainWidget = newTabWidget;
                    } else {
                        delete newTabWidget;
                        newTabWidget = nullptr;
                        moduleProxyParentWidget = configDialog;
                        mainWidget->setParent(nullptr);
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
        QWidget *showWidget = new QWidget(configDialog);
        QVBoxLayout *layout = new QVBoxLayout;
        showWidget->setLayout(layout);
        layout->addWidget(mainWidget);
        layout->insertSpacing(-1, QApplication::style()->pixelMetric(QStyle::PM_DialogButtonsSeparator));

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        connect(buttonBox, &QDialogButtonBox::accepted, configDialog.data(), &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, configDialog.data(), &QDialog::reject);
        layout->addWidget(buttonBox);
        showWidget->adjustSize();

//         connect(&configDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));

        if (configDialog->exec() == QDialog::Accepted) {
            for (KCModuleProxy *moduleProxy: moduleProxyList) {
                moduleProxy->save();
            }
        } else {
            for (KCModuleProxy *moduleProxy: moduleProxyList) {
                moduleProxy->load();
            }
        }

        qDeleteAll(moduleProxyList);
        moduleProxyList.clear();
    }
}

