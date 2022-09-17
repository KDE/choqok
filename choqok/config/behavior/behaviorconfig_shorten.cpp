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
#include <KAboutPluginDialog>
#include <KCMultiDialog>

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

    if (!key.isEmpty() && key != QLatin1String("none") && !availablePlugins.value(key).value(QStringLiteral("X-KDE-ConfigModule")).isEmpty()) {
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
    shortenPlugins->clear();
    shortenPlugins->addItem(i18nc("No shortener service", "None"), QLatin1String("none"));
    for (const auto &plugin : Choqok::PluginManager::self()->availablePlugins(QLatin1String("Shorteners"))) {
        shortenPlugins->addItem(QIcon::fromTheme(plugin.iconName()), plugin.name(), plugin.pluginId());
        availablePlugins.insert(plugin.pluginId(), plugin);
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
    auto metaData = availablePlugins.value(shorten);

    KAboutPluginDialog aboutPlugin(metaData, this);
    aboutPlugin.setWindowIcon(QIcon::fromTheme(metaData.iconName()));
    aboutPlugin.exec();
}

void BehaviorConfig_Shorten::slotConfigureClicked()
{
    auto dialog = new KCMultiDialog(this);
    auto pluginInfo = availablePlugins.value(shortenPlugins->itemData(shortenPlugins->currentIndex()).toString());
    const QString configModuleName = pluginInfo.value(QStringLiteral("X-KDE-ConfigModule"));
    KPluginMetaData md(configModuleName);
    dialog->addModule(md);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

