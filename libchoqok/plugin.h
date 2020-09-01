/*
    This file is part of Choqok, the KDE micro-blogging client
    Some of below codes are got from Kopete source code.

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

#ifndef CHOQOKPLUGIN_H
#define CHOQOKPLUGIN_H

#include <QObject>

#include <KXMLGUIClient>

#include "choqok_export.h"

class KPluginInfo;

/**
* Current Choqok plugin interface version. Interfaces declare plugin version
* to make sure old source (or binary) incompatible plugins are not loaded.
* Increase this if it is necessary that old plugins stop working.
*/
#define CHOQOK_PLUGIN_VERSION 1

namespace Choqok
{

/**
* @brief Base class for all plugins or microblogs.
*
* To create a plugin, you need to create a .desktop file which looks like that:
* \verbatim
[Desktop Entry]
Encoding=UTF-8
Type=Service
X-Choqok-Version=1
Icon=icon
ServiceTypes=Choqok/Plugin
X-KDE-Library=choqok_myplugin
X-KDE-PluginInfo-Author=Your Name
X-KDE-PluginInfo-Email=your@mail.com
X-KDE-PluginInfo-Name=choqok_myplugin
X-KDE-PluginInfo-Version=0.1
X-KDE-PluginInfo-Website=http://yoursite.com
X-KDE-PluginInfo-Category=Plugins
X-KDE-PluginInfo-License=GPL
X-KDE-PluginInfo-EnabledByDefault=false
Name=MyPlugin
Comment=Plugin that do some nice stuff
\endverbatim
*
* The constructor of your plugin should looks like this:
*
* \code
K_PLUGIN_FACTORY_WITH_JSON( MyPluginFactory, "choqok_plugin.json", registerPlugin < MyPlugin > (); )

MyPlugin::MyPlugin( QObject *parent, const char *name, const QList\<QVariant\> &  args  )
: Choqok::Plugin( name, parent )
{
//...
}
\endcode
*
* Choqok::Plugin inherits from KXMLGUIClient.  That client is added
* to the Choqok's mainwindow KXMLGUIFactory. So you may add actions
* on the main window.
* Please note that the client is added right after the plugin is created,
* so you have to create every actions in the constructor.
*
* @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT Plugin : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    Plugin(const QString &componentName, QObject *parent);
    virtual ~Plugin();

    /**
    * Returns the KPluginInfo object associated with this plugin
    */
    KPluginInfo pluginInfo() const;

    /**
    * Get the name of the icon for this plugin. The icon name is taken from the
    * .desktop file.
    *
    * May return an empty string if the .desktop file for this plugin specifies
    * no icon name to use.
    *
    * This is a convenience method that simply calls @ref pluginInfo()->icon().
    */
    QString pluginIcon() const;

    /**
    * Returns the display name of this plugin.
    *
    * This is a convenience method that simply calls @ref pluginInfo()->name().
    */
    QString displayName() const;

    /**
    * @brief Get the plugin id
    * @return the plugin's id which is gotten by calling QObject::metaObject()->className().
    */
    QString pluginId() const;

    /**
    * @brief Returns the pluginName
    * This is a convenience method that simply calls @ref pluginInfo()->pluginName().
    */
    QString pluginName() const;

    /**
    * @brief Prepare for unloading a plugin
    *
    * When unloading a plugin the plugin manager first calls aboutToUnload()
    * to indicate the pending unload. Some plugins need time to shutdown
    * asynchronously and thus can't be simply deleted in the destructor.
    *
    * The default implementation immediately emits the @ref readyForUnload() signal,
    * which basically makes the shutdown immediate and synchronous. If you need
    * more time you can reimplement this method and fire the signal whenever
    * you're ready. (you have 3 seconds)
    *
    * @ref Choqok::MicroBlog reimplement it.
    */
    virtual void aboutToUnload();

Q_SIGNALS:
    /**
    * Notify that the settings of a plugin were changed.
    * These changes are passed on from the new KCDialog code in kdelibs/kutils.
    */
    void settingsChanged();

    /**
     * Indicate when we're ready for unload.
     * @see aboutToUnload()
     */
    void readyForUnload();

private:
    class Private;
    Private *const d;
};

}

#endif // PLUGIN_H
