/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef NOTIFY_H
#define NOTIFY_H

#include <QMap>
#include <QQueue>
#include <QPoint>
#include <QPointer>
#include <QTimer>

#include "plugin.h"

class Notification;
namespace Choqok
{
namespace UI
{
class PostWidget;
}
class Account;
}

class Notify : public Choqok::Plugin
{
    Q_OBJECT
public:
    Notify(QObject *parent, const QList< QVariant > &args);
    ~Notify();

protected Q_SLOTS:
    void slotNewPostWidgetAdded(Choqok::UI::PostWidget *, Choqok::Account *, QString);
    void notifyNextPost();
    void stopNotifications();
    void slotPostReaded();

private:
    void notify(QPointer< Choqok::UI::PostWidget > post);
    void hideLastNotificationAndShowThis(Notification *nextNotificationToShow = nullptr);

    QTimer timer;
    QMap<QString, QStringList> accountsList;
    QQueue<Choqok::UI::PostWidget *> postQueueToNotify;
    Notification *notification;
    QPoint notifyPosition;
};

#endif //NOTIFY_H
