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
#include <KPluginWidget>
#include <ksettings/Dispatcher>
#include <KConfigGroup>

#include "pluginmanager.h"
#include "pluginsdebug.h"

K_PLUGIN_FACTORY_WITH_JSON(ChoqokPluginConfigFactory, "choqok_pluginconfig.json",
                           registerPlugin<ChoqokPluginConfig>();)

ChoqokPluginConfig::ChoqokPluginConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    m_pluginWidget = new KPluginWidget(this);
    m_pluginWidget->setConfig(KSharedConfig::openConfig()->group("Plugins"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_pluginWidget);

    connect(m_pluginWidget, &KPluginWidget::changed, this, &ChoqokPluginConfig::markAsChanged);

    m_pluginWidget->addPlugins(Choqok::PluginManager::self()->availablePlugins(QLatin1String("Plugins")),
                                i18n("General Plugins"));
}

ChoqokPluginConfig::~ChoqokPluginConfig()
{
}

void ChoqokPluginConfig::load()
{
    KCModule::load();
}

void ChoqokPluginConfig::defaults()
{
    m_pluginWidget->defaults();
}

void ChoqokPluginConfig::save()
{
    m_pluginWidget->save();
    Choqok::PluginManager::self()->loadAllPlugins();

    KCModule::save();
}

#include "choqokpluginconfig.moc"
#include "moc_choqokpluginconfig.cpp"
