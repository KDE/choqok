/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "choqokpluginconfig.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginSelector>
#include <ksettings/Dispatcher>

#include "pluginmanager.h"
#include "pluginsdebug.h"

K_PLUGIN_FACTORY_WITH_JSON(ChoqokPluginConfigFactory, "choqok_pluginconfig.json",
                           registerPlugin<ChoqokPluginConfig>();)

ChoqokPluginConfig::ChoqokPluginConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    m_pluginSelector = new KPluginSelector(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_pluginSelector);

    connect(m_pluginSelector, &KPluginSelector::changed, this, &ChoqokPluginConfig::markAsChanged);
    connect(m_pluginSelector, &KPluginSelector::configCommitted,
            this, &ChoqokPluginConfig::reparseConfiguration);

    m_pluginSelector->addPlugins(Choqok::PluginManager::self()->availablePlugins(QLatin1String("Plugins")),
                                 KPluginSelector::ReadConfigFile, i18n("General Plugins"), QLatin1String("Plugins"));
//     m_pluginSelector->addPlugins( Choqok::PluginManager::self()->availablePlugins( "Shorteners" ),
//                                   KPluginSelector::ReadConfigFile, i18n("Shortener Plugins"), "Shorteners");
    m_pluginSelector->load();
}

ChoqokPluginConfig::~ChoqokPluginConfig()
{
}

void ChoqokPluginConfig::reparseConfiguration(const QByteArray &conf)
{
    KSettings::Dispatcher::reparseConfiguration(QLatin1String(conf));
}

void ChoqokPluginConfig::load()
{
    m_pluginSelector->load();

    KCModule::load();
}

void ChoqokPluginConfig::defaults()
{
    m_pluginSelector->defaults();
}

void ChoqokPluginConfig::save()
{
    m_pluginSelector->save();
    Choqok::PluginManager::self()->loadAllPlugins();

    KCModule::save();
}

#include "choqokpluginconfig.moc"

