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
    : KCModule(KAboutData::pluginData(QLatin1String("kcm_choqok_pluginconfig")), parent, args)
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

