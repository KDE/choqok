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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kxmlguiwindow.h>
// #include "datacontainers.h"
#include "account.h"
// #include "ui_prefs_base.h"
// #include "ui_accounts_base.h"
// #include "ui_appears_base.h"
// #include "searchwindow.h"

namespace Choqok {
namespace UI{
    class QuickPost;
}
class Plugin;
}

class QTimer;
class SysTrayIcon;
class KTabWidget;

/**
 * This class serves as the main window for Choqok.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 */

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
    * Default Constructor
    */
    MainWindow();

    /**
    * Default Destructor
    */
    virtual ~MainWindow();

signals:
    void updateTimelines();
//     void updateSearchResults();
//     void updateSearchLimits();
//     void sigSetUnread( int unread );
//     void abortPostNewStatus();
    void markAllAsRead();
    void removeOldPosts();

public slots:
//     void systemNotify( const QString &title, const QString &message, const QString &iconUrl );
//     void search(int type = 0, const QString &search = QString());
    void nextTab(const QWheelEvent&);

protected slots:
    void optionsPreferences();
    void settingsChanged();
    void showStatusMessage( const QString &message, bool isPermanent = false );
    void slotQuit();
    void showBlog();
    void slotUpdateUnreadCount( int change, int sum );

protected:
    void hideEvent( QHideEvent * event );
    QSize sizeHint() const;

private:
    void setupActions();
    void createQuickPostDialog();
    void disableApp();
    void enableApp();
    virtual bool queryExit();
    virtual bool queryClose();

private slots:
    void loadAllAccounts();
    void newPluginAvailable( Choqok::Plugin *plugin );
    void addBlog( Choqok::Account *account, bool isStartup = false );
    void removeBlog( const QString &alias );
    void setTimeLineUpdatesEnabled( bool isEnabled );
    void setNotificationsEnabled( bool isEnabled );
    void triggerQuickPost();
    void toggleMainWindow();
    void slotMarkAllAsRead();
private:
    KTabWidget *mainWidget;
    QTimer *timelineTimer;
    int mPrevUpdateInterval;
    SysTrayIcon *sysIcon;
    Choqok::UI::QuickPost *quickWidget;
};

#endif
