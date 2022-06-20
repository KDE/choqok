/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QDBusInterface>
#include <QJsonArray>
#include <QTimer>

#include <KPluginFactory>

#include <Purpose/PluginBase>

#include "choqokinterface.h"
EXPORT_SHARE_VERSION

class ChoqokJob : public Purpose::Job
{
    Q_OBJECT
    public:
        ChoqokJob(QObject* parent)
            : Purpose::Job(parent)
        {}

        QStringList arrayToList(const QJsonArray& array)
        {
            QStringList ret;
            for (const QJsonValue& val : array) {
                ret += val.toString();
            }
            return ret;
        }

        void start() override
        {
            OrgKdeChoqokInterface iface(QStringLiteral("org.kde.choqok"), QStringLiteral("/"), QDBusConnection::sessionBus(), this);
            const QStringList urls = arrayToList(data().value(QStringLiteral("urls")).toArray());

            for (const QString &url : urls) {
                iface.uploadFile(url);
            }

            QTimer::singleShot(0, this, [this]() {
                emitResult();
            });
        }
};


class Q_DECL_EXPORT PurposePlugin : public Purpose::PluginBase
{

    Q_OBJECT
public:
    PurposePlugin(QObject* p, const QVariantList& ) : Purpose::PluginBase(p) {}

    Purpose::Job* createJob() const override
    {
        return new ChoqokJob(nullptr);
    }

};

K_PLUGIN_CLASS_WITH_JSON(PurposePlugin, "purposeplugin.json")

#include "purposeplugin.moc"
