/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

const QString baseText(QLatin1String("<table height=\"100%\" width=\"100%\"><tr><td rowspan=\"2\"\
width=\"48\"><img src=\"img://profileImage\" width=\"48\" height=\"48\" /></td><td width=\"5\"><!-- EMPTY HAHA --></td><td><b>%1 :</b><a href='choqok://close'><img src='icon://close' title='%4' align='right' /></a><div dir=\"%3\">%2</div></td></tr></table>"));

class NotifySettings : public QObject
{

public:
    NotifySettings(QObject *parent = nullptr);
    virtual ~NotifySettings();

    QMap<QString, QStringList> accounts();
    void setAccounts(QMap<QString, QStringList> accounts);

    int notifyInterval() const;
    void setNotifyInterval(int interval);

    QPoint position() const;
    void setPosition(const QPoint &position);

    QColor foregroundColor() const;
    void setForegroundColor(const QColor &color);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    QFont font() const;
    void setFont(const QFont &font);

    void load();
    void save();

private:
    class Private;
    Private *const d;
};

#endif // NOTIFYSETTINGS_H
