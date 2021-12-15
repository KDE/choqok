/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "plugin.h"

#include <KPluginInfo>
#include <ksettings/Dispatcher>

#include "pluginmanager.h"

namespace Choqok
{

class Plugin::Private
{
public:

};

Plugin::Plugin(const QString &componentName, QObject *parent)
    : QObject(parent), KXMLGUIClient(), d(new Private)
{
    //setComponentData( instance );
    setComponentName(componentName, componentName);
    KSettings::Dispatcher::registerComponent(componentName, this, "settingsChanged");
}

Plugin::~Plugin()
{
    delete d;
}

QString Plugin::pluginId() const
{
    return QLatin1String(metaObject()->className());
}

QString Plugin::displayName() const
{
    return pluginInfo().isValid() ? pluginInfo().name() : QString();
}

QString Plugin::pluginName() const
{
    return pluginInfo().isValid() ? pluginInfo().pluginName() : QString();
}

QString Plugin::pluginIcon() const
{
    return pluginInfo().isValid() ? pluginInfo().icon() : QString();
}

KPluginInfo Plugin::pluginInfo() const
{
    return PluginManager::self()->pluginInfo(this);
}

void Plugin::aboutToUnload()
{
    // Just make the unload synchronous by default
    Q_EMIT readyForUnload();
}

}

