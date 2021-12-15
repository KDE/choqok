/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    SysTrayIcon(Choqok::UI::MainWindow *parent);
    ~SysTrayIcon();
    int unreadCount() const;

public Q_SLOTS:
    void setTimeLineUpdatesEnabled(bool isEnabled);
    void slotJobDone(Choqok::JobResult result);
    void updateUnreadCount(int changeOfUnreadPosts);
    void resetUnreadCount();

protected Q_SLOTS:
    void slotRestoreIcon();

private:
    QString currentIconName();
    int unread;

    Choqok::UI::MainWindow *_mainwin;
    bool isOffline;
};

#endif
