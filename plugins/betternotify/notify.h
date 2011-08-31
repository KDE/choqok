/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef NOTIFY_H
#define NOTIFY_H

#include <plugin.h>
#include <qqueue.h>
#include <KUrl>
#include <QPointer>
#include <QTimer>
#include <QPoint>

class Notification;
namespace Choqok {
namespace UI {
    class PostWidget;
}
class Account;
}

class Notify : public Choqok::Plugin
{
    Q_OBJECT
public:
    Notify( QObject* parent, const QList< QVariant >& args );
    ~Notify();

protected slots:
    void slotNewPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString);
    void notifyNextPost();
    void stopNotifications();
    void slotPostReaded();

private:
    void notify( QPointer< Choqok::UI::PostWidget > post );
    void hideLastNotificationAndShowThis(Notification* nextNotificationToShow = 0);

    QTimer timer;
    QMap<QString, QStringList> accountsList;
    QQueue<Choqok::UI::PostWidget*> postQueueToNotify;
    Notification *notification;
    QPoint notifyPosition;
};

#endif //NOTIFY_H
