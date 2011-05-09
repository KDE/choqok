/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2011  Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

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

#include <QtCore/QObject>
#include <QStringList>


class NotifySettings : public QObject
{

public:
    NotifySettings(QObject* parent = 0);
    virtual ~NotifySettings();
    QMap<QString, QStringList> accounts();
    void setAccounts(QMap<QString, QStringList> accounts);

    void load();
    void save();

private:
    class Private;
    Private * const d;
};

#endif // NOTIFYSETTINGS_H
