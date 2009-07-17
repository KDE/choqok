/*
    This file is part of Choqok, the KDE micro-blogging client
    Some of below codes are got from Kopete source code.

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef CHOQOKPLUGINMANAGER_H
#define CHOQOKPLUGINMANAGER_H

#include <kdemacros.h>
#include <QtCore/QObject>
#include <QtCore/QList>

class KPluginInfo;

namespace Choqok
{

class Plugin;
class Protocol;
typedef QList<Plugin*> PluginList;
class PluginManagerPrivate;

/**
 * @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 */
class KDE_EXPORT PluginManager : public QObject
{
    friend class PluginManagerPrivate;
    Q_OBJECT
    Q_ENUMS( PluginLoadMode )

public:
    /**
     * Retrieve the plugin loader instance.
     */
    static PluginManager* self();

    /**
     * Returns a list of all available plugins for the given category.
     * Currently there are two categories, "Plugins" and "Protocols", but
     * you can add your own categories if you want.
     *
     * If you pass an empty string you get the complete list of ALL plugins.
     *
     * You can query all information on the plugins through the KPluginInfo
     * interface.
     */
    QList<KPluginInfo> availablePlugins( const QString &category = QString() ) const;

    /**
     * Returns a list of all plugins that are actually loaded.
     * If you omit the category you get all, otherwise it's a filtered list.
     * See also @ref availablePlugins().
     */
    PluginList loadedPlugins( const QString &category = QString() ) const;

    /**
     * @brief Search by plugin name. This is the key used as X-KDE-PluginInfo-Name in
     * the .desktop file.
     *
     * @return The @ref Choqok::Plugin object found by the search, or a null
     * pointer if the plugin is not loaded.
     *
     * If you want to also load the plugin you can better use @ref loadPlugin, which returns
     * the pointer to the plugin if it's already loaded.
     */
    Plugin *plugin( const QString &pluginName ) const;

    /**
     * @return the KPluginInfo for the specified plugin
     */
    KPluginInfo pluginInfo( const Choqok::Plugin *plugin ) const;


    /**
     * Shuts down the plugin manager on Choqok shutdown, but first
     * unloads all plugins asynchronously.
     *
     * After 3 seconds all plugins should be removed; what's still left
     * by then is unloaded through a hard delete instead.
     *
     * Note that this call also derefs the plugin manager from the event
     * loop, so do NOT call this method when not terminating Choqok!
     */
    void shutdown();

    /**
     * Enable a plugin.
     *
     * This marks a plugin as enabled in the config file, so loadAll()
     * can pick it up later.
     *
     * This method does not actually load a plugin, it only edits the
     * config file.
     *
     * @param name is the name of the plugin as it is listed in the .desktop
     * file in the X-KDE-Library field.
     * @param enabled sets whether or not the plugin is enabled
     *
     * Returns false when no appropriate plugin can be found.
     */
    bool setPluginEnabled( const QString &name, bool enabled = true );

    /**
     * This method check if all the plugins are loaded.
     * @return true if all the plugins are loaded.
     */
    bool isAllPluginsLoaded() const;

    /**
     * Plugin loading mode. Used by @ref loadPlugin(). Code that doesn't want to block
     * the GUI and/or lot a lot of plugins at once should use asynchronous loading (@c LoadAsync).
     * The default is synchronous loading (@c LoadSync).
     */
    enum PluginLoadMode { LoadSync, LoadAsync };

public slots:
    /**
     * @brief Load a single plugin by plugin name. Returns an existing plugin
     * if one is already loaded in memory.
     *
     * If mode is set to Async, the plugin will be queued and loaded in
     * the background. This method will return a null pointer. To get
     * the loaded plugin you can track the @ref pluginLoaded() signal.
     *
     * See also @ref plugin().
     */
    Plugin *loadPlugin( const QString &pluginId, PluginLoadMode mode = LoadSync );

    /**
     * @brief Unload the plugin specified by @p pluginName
     */
    bool unloadPlugin( const QString &pluginName );

    /**
     * @brief Loads all the enabled plugins. Also used to reread the
     * config file when the configuration has changed.
     */
    void loadAllPlugins();

signals:
    /**
     * @brief Signals a new plugin has just been loaded.
     */
    void pluginLoaded( Choqok::Plugin *plugin );

    /**
     * @brief Signals a plugin has just been unloaded.
     */
    void pluginUnloaded( const QString &pluginName );

    /**
     * @brief Signals a new protocol has just been loaded.
     * @note pluginLoaded is also emitted before this signal
     */
    void protocolLoaded( Choqok::Protocol *protocol );

    /**
     * @brief All plugins have been loaded by the plugin manager.
     *
     * This signal is emitted exactly ONCE, when the plugin manager has emptied
     * its plugin queue for the first time. This means that if you call an async
     * loadPlugin() before loadAllPlugins() this signal is probably emitted after
     * the initial call completes, unless you are quick enough to fill the queue
     * before it completes, which is a dangerous race you shouldn't count upon :)
     *
     * The signal is delayed one event loop iteration through a singleShot timer,
     * but that is not guaranteed to be enough for account instantiation. You may
     * need an additional timer for it in the code if you want to programmatically
     * act on it.
     *
     * If you use the signal for enabling/disabling GUI objects there is little
     * chance a user is able to activate them in the short while that's remaining,
     * the slow part of the code is over now and the remaining processing time
     * is neglectable for the user.
     */
    void allPluginsLoaded();

private slots:
    /**
     * @brief Cleans up some references if the plugin is destroyed
     */
    void slotPluginDestroyed( QObject *plugin );

    /**
     * shutdown() starts a timer, when it fires we force all plugins
     * to be unloaded here by deref()-ing the event loop to trigger the plugin
     * manager's destruction
     */
    void slotShutdownTimeout();

    /**
     * Common entry point to deref() the KApplication. Used both by the clean
     * shutdown and the timeout condition of slotShutdownTimeout()
     */
    void slotShutdownDone();

    /**
     * Emitted by a Choqok::Plugin when it's ready for unload
     */
    void slotPluginReadyForUnload();

    /**
     * Load a plugin from our queue. Does nothing if the queue is empty.
     * Schedules itself again if more plugins are pending.
     */
    void slotLoadNextPlugin();

private:
    /**
     * @internal
     *
     * The internal method for loading plugins.
     * Called by @ref loadPlugin directly or through the queue for async plugin
     * loading.
     */
    Plugin * loadPluginInternal( const QString &pluginId );

    /**
     * @internal
     *
     * Find the KPluginInfo structure by key. Reduces some code duplication.
     *
     * Returns a null pointer when no plugin info is found.
     */
    KPluginInfo infoForPluginId( const QString &pluginId ) const;

    PluginManager();
    ~PluginManager();

};

}

#endif // KOPETEPLUGINMANAGER_H

// vim: set noet ts=4 sts=4 sw=4:
