/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#ifndef SYSTRAYICON_H
#define SYSTRAYICON_H

#include <ksystemtrayicon.h>
#include "mainwindow.h"
#include "quicktwit.h"

/**
System tray icon!

	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class SysTrayIcon : public KSystemTrayIcon
{
	Q_OBJECT
public:
    SysTrayIcon(QWidget* parent=0);

    ~SysTrayIcon();
public slots:
	void quitApp();
	void postQuickTwit();
	void toggleMainWindowVisibility();
	void sysTrayActivated( QSystemTrayIcon::ActivationReason reason );
	void systemNotify(const QString &title, const QString &message, const QString &iconUrl);
	
protected slots:
	void slotSetUnread(int unread);
	
private:
	void setupActions();
	
	MainWindow *mainWin;
	QuickTwit *quickWidget;
// 	bool isQuickActivated;
	
	QPixmap m_defaultIcon;
	int m_unread;
};

#endif
