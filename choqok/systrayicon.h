/*
    This file is part of Choqok, the KDE micro-blogging client

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
#ifndef SYSTRAYICON_H
#define SYSTRAYICON_H

#include <KStatusNotifierItem>

#include "choqoktypes.h"
#include "mainwindow.h"

/**
System tray icon!

    @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class SysTrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
public:
    SysTrayIcon( Choqok::UI::MainWindow* parent );
    ~SysTrayIcon();
    int unreadCount() const;

public Q_SLOTS:
    void setTimeLineUpdatesEnabled( bool isEnabled );
    void slotJobDone( Choqok::JobResult result );
    void updateUnreadCount( int changeOfUnreadPosts );
    void resetUnreadCount();

protected Q_SLOTS:
    void slotRestoreIcon();

private:
    QString currentIconName();
    int unread;

    Choqok::UI::MainWindow * _mainwin;
    bool isOffline;
};

#endif
