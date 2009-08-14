/*
    This file is part of Choqok, the KDE micro-blogging client

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
#ifndef SYSTRAYICON_H
#define SYSTRAYICON_H

#include <ksystemtrayicon.h>
#include <choqoktypes.h>
// #include "mainwindow.h"
// #include "quicktwit.h"

/**
System tray icon!

    @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class SysTrayIcon : public KSystemTrayIcon
{
    Q_OBJECT
public:
    SysTrayIcon( QWidget* parent = 0 );

    ~SysTrayIcon();
public slots:
    void setTimeLineUpdatesEnabled( bool isEnabled );
    void slotJobDone( Choqok::JobResult result );
    void updateUnreadCount( int changeOfUnreadPosts );
    void resetUnreadCount();

signals:
    void wheelEvent(const QWheelEvent&);

protected slots:
    void slotRestoreIcon();

protected:
    virtual bool event(QEvent* event);

private:
    int unread;

    QPixmap m_defaultIcon;
    QIcon prevIcon;
    bool isIconChanged;
    bool isBaseIconChanged;
};

#endif
