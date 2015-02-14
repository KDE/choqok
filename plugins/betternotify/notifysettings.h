/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2011-2012  Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef NOTIFYSETTINGS_H
#define NOTIFYSETTINGS_H

#include <QColor>
#include <QFont>
#include <QObject>
#include <QPoint>
#include <QStringList>

#define NOTIFICATION_WIDTH 300
#define NOTIFICATION_HEIGHT 70

const QString baseText( "<table height=\"100%\" width=\"100%\"><tr><td rowspan=\"2\"\
width=\"48\"><img src=\"img://profileImage\" width=\"48\" height=\"48\" /></td><td width=\"5\"><!-- EMPTY HAHA --></td><td><b>%1 :</b><a href='choqok://close'><img src='icon://close' title='%4' align='right' /></a><div dir=\"%3\">%2</div></td></tr></table>" );

class NotifySettings : public QObject
{

public:
    NotifySettings(QObject* parent = 0);
    virtual ~NotifySettings();

    QMap<QString, QStringList> accounts();
    void setAccounts(QMap<QString, QStringList> accounts);

    int notifyInterval() const;
    void setNotifyInterval(int interval);

    QPoint position() const;
    void setPosition(const QPoint& position);

    QColor foregroundColor() const;
    void setForegroundColor(const QColor& color);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);

    QFont font() const;
    void setFont(const QFont& font);

    void load();
    void save();

private:
    class Private;
    Private * const d;
};

#endif // NOTIFYSETTINGS_H
