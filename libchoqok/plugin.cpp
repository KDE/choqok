/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "plugin.h"

#include <KPluginMetaData>
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

QString Plugin::displayName() const
{
    return pluginMetaData().name();
}

QString Plugin::pluginId() const
{
    return pluginMetaData().pluginId();
}

QString Plugin::pluginIcon() const
{
    return pluginMetaData().iconName();
}

KPluginMetaData Plugin::pluginMetaData() const
{
    return PluginManager::self()->pluginMetaData(this);
}

void Plugin::aboutToUnload()
{
    // Just make the unload synchronous by default
    Q_EMIT readyForUnload();
}

}

