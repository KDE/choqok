/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "notify.h"

#include <QDesktopWidget>

#include <KPluginFactory>

#include "account.h"
#include "application.h"
#include "choqokuiglobal.h"
#include "mediamanager.h"
#include "postwidget.h"

#include "notification.h"
#include "notifysettings.h"

K_PLUGIN_CLASS_WITH_JSON(Notify, "choqok_notify.json")

Notify::Notify(QObject *parent, const QList< QVariant > &)
    : Choqok::Plugin(QLatin1String("choqok_betternotify"), parent), notification(nullptr)
{
    NotifySettings set;
    accountsList = set.accounts();
    timer.setInterval(set.notifyInterval() * 1000);
    connect(Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::newPostWidgetAdded,
            this, &Notify::slotNewPostWidgetAdded);
    connect(&timer, &QTimer::timeout, this, &Notify::notifyNextPost);

    notifyPosition = set.position();
}

Notify::~Notify()
{
}

void Notify::slotNewPostWidgetAdded(Choqok::UI::PostWidget *pw, Choqok::Account *acc, QString tm)
{
//     qDebug()<<Choqok::Application::isStartingUp()<< Choqok::Application::isShuttingDown();
    if (Choqok::Application::isStartingUp() || Choqok::Application::isShuttingDown()) {
        //qDebug()<<"Choqok is starting up or going down!";
        return;
    }
    if (pw && !pw->isRead() && accountsList[acc->alias()].contains(tm)) {
        //qDebug()<<"POST ADDED TO NOTIFY IT: "<<pw->currentPost()->content;
        postQueueToNotify.enqueue(pw);
        if (!timer.isActive()) {
            notifyNextPost();
            timer.start();
        }
    }
}

void Notify::notifyNextPost()
{
    if (postQueueToNotify.isEmpty()) {
        timer.stop();
        if (notification) {
            hideLastNotificationAndShowThis();
        }
    } else {
        notify(postQueueToNotify.dequeue());
    }
}

void Notify::notify(QPointer< Choqok::UI::PostWidget > post)
{
    if (post) {
        Notification *notif = new Notification(post);
        connect(notif, &Notification::ignored, this, &Notify::stopNotifications);
        connect(notif, &Notification::postReaded, this, &Notify::slotPostReaded);
        connect(notif, &Notification::mouseEntered, &timer, &QTimer::stop);
        connect(notif, &Notification::mouseLeaved, &timer, (void (QTimer::*)())&QTimer::start);
        hideLastNotificationAndShowThis(notif);
    } else  {
        hideLastNotificationAndShowThis();
    }
}

void Notify::slotPostReaded()
{
    notifyNextPost();
    timer.stop();
    timer.start();
}

void Notify::stopNotifications()
{
    postQueueToNotify.clear();
    timer.stop();
    hideLastNotificationAndShowThis();
}

void Notify::hideLastNotificationAndShowThis(Notification *nextNotificationToShow)
{
    //TODO: Add Animation
    notification->deleteLater();
    notification = nullptr;
    if (nextNotificationToShow) {
        notification = nextNotificationToShow;
        notification->move(notifyPosition);
        notification->show();
    }
}

#include "moc_notify.cpp"
#include "notify.moc"
