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

#include "plugin.h"

#include <KPluginInfo>
#include <KSettings/Dispatcher>

#include "pluginmanager.h"

namespace Choqok {

class Plugin::Private
{
public:
    
};

Plugin::Plugin( const KComponentData &instance, QObject *parent )
: QObject( parent ), KXMLGUIClient(), d(new Private)
{
    setComponentData( instance );
    KSettings::Dispatcher::registerComponent( instance, this, "settingsChanged" );
}

Plugin::~Plugin()
{
    delete d;
}

QString Plugin::pluginId() const
{
    return QString::fromLatin1( metaObject()->className() );
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
    return PluginManager::self()->pluginInfo( this );
}

void Plugin::aboutToUnload()
{
    // Just make the unload synchronous by default
    Q_EMIT readyForUnload();
}

}

#include "plugin.moc"
