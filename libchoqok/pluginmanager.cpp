/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pluginmanager.h"

#include <QRegExp>
#include <QTimer>
#include <QStack>
#include <QCoreApplication>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KPluginMetaData>
#include <KPluginFactory>

#include "accountmanager.h"
#include "libchoqokdebug.h"

namespace Choqok
{

class PluginManagerPrivate
{
public:
    PluginManagerPrivate() : shutdownMode(StartingUp), isAllPluginsLoaded(false)
    {
        plugins = KPluginMetaData::findPlugins(QStringLiteral("choqok_plugins"));
    }

    ~PluginManagerPrivate()
    {
        if (shutdownMode != DoneShutdown) {
            qCWarning(CHOQOK) << "Destructing plugin manager without going through the shutdown process!" << endl;
        }

        // Clean up loadedPlugins manually, because PluginManager can't access our global
        // static once this destructor has started.
        for (const KPluginMetaData &p: loadedPlugins.keys()) {
            Plugin *plugin = loadedPlugins.value(p);
            qCWarning(CHOQOK) << "Deleting stale plugin '" << plugin->pluginId() << "'";
            plugin->disconnect(&instance, SLOT(slotPluginDestroyed(QObject*)));
            plugin->deleteLater();;
            loadedPlugins.remove(p);
        }
    }

    // All available plugins, regardless of category, and loaded or not
    QVector<KPluginMetaData> plugins;

    // Dict of all currently loaded plugins, mapping the KPluginMetaData to
    // a plugin
    typedef QHash<KPluginMetaData, Plugin *> MetaDataToPluginMap;
    MetaDataToPluginMap loadedPlugins;

    // The plugin manager's mode. The mode is StartingUp until loadAllPlugins()
    // has finished loading the plugins, after which it is set to Running.
    // ShuttingDown and DoneShutdown are used during Choqok shutdown by the
    // async unloading of plugins.
    enum ShutdownMode { StartingUp, Running, ShuttingDown, DoneShutdown };
    ShutdownMode shutdownMode;

    // Plugins pending for loading
    QStack<QString> pluginsToLoad;

    bool isAllPluginsLoaded;
    PluginManager instance;
};

Q_GLOBAL_STATIC(PluginManagerPrivate, _kpmp)

PluginManager *PluginManager::self()
{
    return &_kpmp->instance;
}

PluginManager::PluginManager() : QObject()
{
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
            this, &PluginManager::slotAboutToQuit);
}

PluginManager::~PluginManager()
{
}

QVector<KPluginMetaData> PluginManager::availablePlugins(const QString &category) const
{
    if (category.isEmpty()) {
        return _kpmp->plugins;
    }

    QVector<KPluginMetaData> result;
    for (const KPluginMetaData &p: _kpmp->plugins) {
        if (p.category().compare(category) == 0) {
            result.append(p);
        }
    }

    return result;
}

PluginList PluginManager::loadedPlugins(const QString &category) const
{
    PluginList result;

    for (const KPluginMetaData &p: _kpmp->loadedPlugins.keys()) {
        if (category.isEmpty() || p.category().compare(category) == 0) {
            result.append(_kpmp->loadedPlugins.value(p));
        }
    }

    return result;
}

KPluginMetaData PluginManager::pluginMetaData(const Plugin *plugin) const
{
    for (const KPluginMetaData &p: _kpmp->loadedPlugins.keys()) {
        if (_kpmp->loadedPlugins.value(p) == plugin) {
            return p;
        }
    }

    return KPluginMetaData();
}

void PluginManager::shutdown()
{
    qCDebug(CHOQOK);
    if (_kpmp->shutdownMode != PluginManagerPrivate::Running) {
        qCDebug(CHOQOK) << "called when not running.  / state =" << _kpmp->shutdownMode;
        return;
    }

    _kpmp->shutdownMode = PluginManagerPrivate::ShuttingDown;

    // Remove any pending plugins to load, we're shutting down now :)
    _kpmp->pluginsToLoad.clear();

    // Ask all plugins to unload
    for (PluginManagerPrivate::MetaDataToPluginMap::ConstIterator it = _kpmp->loadedPlugins.constBegin();
            it != _kpmp->loadedPlugins.constEnd(); /* EMPTY */) {
        // Plugins could emit their ready for unload signal directly in response to this,
        // which would invalidate the current iterator. Therefore, we copy the iterator
        // and increment it beforehand.
        PluginManagerPrivate::MetaDataToPluginMap::ConstIterator current(it);
        ++it;
        // FIXME: a much cleaner approach would be to just delete the plugin now. if it needs
        //  to do some async processing, it can grab a reference to the app itself and create
        //  another object to do it.
        current.value()->aboutToUnload();
    }

    // When running under valgrind, don't enable the timer because it will almost
    // certainly fire due to valgrind's much slower processing
#if defined(HAVE_VALGRIND_H) && !defined(NDEBUG) && defined(__i386__)
    if (RUNNING_ON_VALGRIND) {
        qCDebug(CHOQOK) << "Running under valgrind, disabling plugin unload timeout guard";
    } else
#endif
        QTimer::singleShot(3000, this, SLOT(slotShutdownTimeout()));
}

void PluginManager::slotPluginReadyForUnload()
{
    qCDebug(CHOQOK);
    // Using QObject::sender() is on purpose here, because otherwise all
    // plugins would have to pass 'this' as parameter, which makes the API
    // less clean for plugin authors
    // FIXME: I don't buy the above argument. Add a Choqok::Plugin::emitReadyForUnload(void),
    //        and make readyForUnload be passed a plugin. - Richard
    Plugin *plugin = dynamic_cast<Plugin *>(const_cast<QObject *>(sender()));
    if (!plugin) {
        qCWarning(CHOQOK) << "Calling object is not a plugin!";
        return;
    }
    qCDebug(CHOQOK) << plugin->pluginId() << "ready for unload";
    _kpmp->loadedPlugins.remove(_kpmp->loadedPlugins.key(plugin));
    plugin->deleteLater();
    plugin = nullptr;
    if (_kpmp->loadedPlugins.count() < 1) {
        slotShutdownDone();
    }
}

void PluginManager::slotShutdownTimeout()
{
    qCDebug(CHOQOK);
    // When we were already done the timer might still fire.
    // Do nothing in that case.
    if (_kpmp->shutdownMode == PluginManagerPrivate::DoneShutdown) {
        return;
    }

    QStringList remaining;
    for (Plugin *p: _kpmp->loadedPlugins.values()) {
        remaining.append(p->pluginId());
    }

    qCWarning(CHOQOK) << "Some plugins didn't shutdown in time!" << endl
                      << "Remaining plugins:" << remaining << endl
                      << "Forcing Choqok shutdown now." << endl;

    slotShutdownDone();
}

void PluginManager::slotShutdownDone()
{
    qCDebug(CHOQOK) ;
    _kpmp->shutdownMode = PluginManagerPrivate::DoneShutdown;
}

void PluginManager::loadAllPlugins()
{
    qCDebug(CHOQOK);
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    if (config->hasGroup(QLatin1String("Plugins"))) {
        QMap<QString, bool> pluginsMap;

        const QMap<QString, QString> entries = config->entryMap(QLatin1String("Plugins"));
        for (const QString &key: entries.keys()) {
            if (key.endsWith(QLatin1String("Enabled"))) {
                pluginsMap.insert(key.left(key.length() - 7), (entries.value(key).compare(QLatin1String("true")) == 0));
            }
        }

        for (const KPluginMetaData &p: availablePlugins(QString())) {
            if ((p.category().compare(QLatin1String("MicroBlogs")) == 0) ||
                    (p.category().compare(QLatin1String("Shorteners")) == 0))
            {
                continue;
            }

            const QString pluginName = p.pluginId();
            if (pluginsMap.value(pluginName, p.isEnabledByDefault())) {
                if (!plugin(pluginName)) {
                    _kpmp->pluginsToLoad.push(pluginName);
                }
            } else {
                //This happens if the user unloaded plugins with the config plugin page.
                // No real need to be assync because the user usually unload few plugins
                // compared tto the number of plugin to load in a cold start. - Olivier
                if (plugin(pluginName)) {
                    unloadPlugin(pluginName);
                }
            }
        }
    } else {
        // we had no config, so we load any plugins that should be loaded by default.
        for (const KPluginMetaData &p: availablePlugins(QString())) {
            if ((p.category().compare(QLatin1String("MicroBlogs")) == 0) ||
                    (p.category().compare(QLatin1String("Shorteners")) == 0))
            {
                continue;
            }

            if (p.isEnabledByDefault()) {
                _kpmp->pluginsToLoad.push(p.pluginId());
            }
        }
    }
    // Schedule the plugins to load
    QTimer::singleShot(0, this, SLOT(slotLoadNextPlugin()));
}

void PluginManager::slotLoadNextPlugin()
{
    qCDebug(CHOQOK);
    if (_kpmp->pluginsToLoad.isEmpty()) {
        if (_kpmp->shutdownMode == PluginManagerPrivate::StartingUp) {
            _kpmp->shutdownMode = PluginManagerPrivate::Running;
            _kpmp->isAllPluginsLoaded = true;
            qCDebug(CHOQOK) << "All plugins loaded...";
            Q_EMIT allPluginsLoaded();
        }
        return;
    }

    QString key = _kpmp->pluginsToLoad.pop();
    loadPluginInternal(key);

    // Schedule the next run unconditionally to avoid code duplication on the
    // allPluginsLoaded() signal's handling. This has the added benefit that
    // the signal is delayed one event loop, so the accounts are more likely
    // to be instantiated.
    QTimer::singleShot(0, this, SLOT(slotLoadNextPlugin()));
}

Plugin *PluginManager::loadPlugin(const QString &_pluginId, PluginLoadMode mode /* = LoadSync */)
{
    QString pluginId = _pluginId;

    // Try to find legacy code
    // FIXME: Find any cases causing this, remove them, and remove this too - Richard
    if (pluginId.endsWith(QLatin1String(".desktop"))) {
        qCWarning(CHOQOK) << "Trying to use old-style API!" << endl;
        pluginId = pluginId.remove(QRegExp(QLatin1String(".desktop$")));
    }

    if (mode == LoadSync) {
        return loadPluginInternal(pluginId);
    } else {
        _kpmp->pluginsToLoad.push(pluginId);
        QTimer::singleShot(0, this, SLOT(slotLoadNextPlugin()));
        return nullptr;
    }
}

Plugin *PluginManager::loadPluginInternal(const QString &pluginId)
{
    qCDebug(CHOQOK) << "Loading Plugin:" << pluginId;

    KPluginMetaData metaData = metaDataForPluginId(pluginId);

    if (_kpmp->loadedPlugins.contains(metaData)) {
        return _kpmp->loadedPlugins[ metaData ];
    }

    auto pluginMetaData = KPluginMetaData::findPluginById(QStringLiteral("choqok_plugins"), pluginId);
    if (!pluginMetaData.isValid()) {
        return nullptr;
    }

    auto pluginResult = KPluginFactory::instantiatePlugin<Plugin>(pluginMetaData);
    if (auto plugin = pluginResult.plugin) {
        _kpmp->loadedPlugins.insert(metaData, plugin);

        connect(plugin, &Plugin::destroyed, this, &PluginManager::slotPluginDestroyed);
        connect(plugin, &Plugin::readyForUnload, this, &PluginManager::slotPluginReadyForUnload);

        qCDebug(CHOQOK) << "Successfully loaded plugin '" << pluginId << "'";

        if (plugin->pluginMetaData().category() != QLatin1String("MicroBlogs") && plugin->pluginMetaData().category() != QLatin1String("Shorteners")) {
            qCDebug(CHOQOK) << "Emitting pluginLoaded()";
            Q_EMIT pluginLoaded(plugin);
        }
        return plugin;
    } else {
        qCDebug(CHOQOK) << "Loading plugin" << pluginId << "failed:" << pluginResult.errorString << pluginResult.errorText;
    }
    return nullptr;
}

bool PluginManager::unloadPlugin(const QString &spec)
{
    qCDebug(CHOQOK) << spec;
    if (Plugin *thePlugin = plugin(spec)) {
        qCDebug(CHOQOK) << "Unloading" << spec;
        thePlugin->aboutToUnload();
        return true;
    } else {
        return false;
    }
}

void PluginManager::slotPluginDestroyed(QObject *plugin)
{
    qCDebug(CHOQOK);
    for (const KPluginMetaData &p: _kpmp->loadedPlugins.keys()) {
        if (_kpmp->loadedPlugins.value(p) == plugin) {
            const QString pluginName = p.name();
            _kpmp->loadedPlugins.remove(p);
            Q_EMIT pluginUnloaded(pluginName);
            break;
        }
    }

    if (_kpmp->shutdownMode == PluginManagerPrivate::ShuttingDown && _kpmp->loadedPlugins.isEmpty()) {
        // Use a timer to make sure any pending deleteLater() calls have
        // been handled first
        QTimer::singleShot(0, this, SLOT(slotShutdownDone()));
    }
}

Plugin *PluginManager::plugin(const QString &_pluginId) const
{
    // Hack for compatibility with Plugin::pluginId(), which returns
    // classname() instead of the internal name. Changing that is not easy
    // as it invalidates the config file, the contact list, and most likely
    // other code as well.
    // For now, just transform FooProtocol to choqok_foo.
    // FIXME: In the future we'll need to change this nevertheless to unify
    //        the handling - Martijn
    QString pluginId = _pluginId;
    if (pluginId.endsWith(QLatin1String("Protocol"))) {
        pluginId = QLatin1String("choqok_") + _pluginId.toLower().remove(QLatin1String("protocol"));
    }
    // End hack

    KPluginMetaData metaData = metaDataForPluginId(pluginId);
    if (!metaData.isValid()) {
        return nullptr;
    }

    if (_kpmp->loadedPlugins.contains(metaData)) {
        return _kpmp->loadedPlugins[ metaData ];
    } else {
        return nullptr;
    }
}

KPluginMetaData PluginManager::metaDataForPluginId(const QString &pluginId) const
{
    for (const KPluginMetaData &p: _kpmp->plugins) {
        if (p.pluginId().compare(pluginId) == 0) {
            return p;
        }
    }

    return KPluginMetaData();
}

bool PluginManager::setPluginEnabled(const QString &_pluginId, bool enabled /* = true */)
{
    QString pluginId = _pluginId;

    KConfigGroup config(KSharedConfig::openConfig(), "Plugins");

    // FIXME: What is this for? This sort of thing is kconf_update's job - Richard
    if (!pluginId.startsWith(QLatin1String("choqok_"))) {
        pluginId.prepend(QLatin1String("choqok_"));
    }

    if (!metaDataForPluginId(pluginId).isValid()) {
        return false;
    }

    config.writeEntry(pluginId + QLatin1String("Enabled"), enabled);
    config.sync();

    return true;
}

bool PluginManager::isAllPluginsLoaded() const
{
    return _kpmp->isAllPluginsLoaded;
}

void PluginManager::slotAboutToQuit()
{
    shutdown();
}

} //END namespace Choqok

#include "moc_pluginmanager.cpp"
