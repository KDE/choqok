/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "account.h"
#include "choqokmainwindow.h"

class KAction;
class KSplashScreen;
namespace Choqok
{
namespace UI
{
    class QuickPost;
    class MicroBlogWidget;
}
    class Plugin;
}
namespace KSettings
{
    class Dialog;
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

class MainWindow : public Choqok::UI::MainWindow
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

protected:
    virtual void hideEvent(QHideEvent* event);
    virtual void showEvent(QShowEvent* );

private slots:
    void nextTab(int delta, Qt::Orientation orientation);
    void loadAllAccounts();
    void newPluginAvailable( Choqok::Plugin *plugin );
    void addBlog( Choqok::Account *account, bool isStartup = false );
    void removeBlog( const QString &alias );
    void setTimeLineUpdatesEnabled( bool isEnabled );
    void setNotificationsEnabled( bool isEnabled );
    void triggerQuickPost();
    void toggleMainWindow();
    void slotMarkAllAsRead();
    void slotUpdateTimelines();
    void slotUploadMedium();

    void slotAppearanceConfigChanged();
    void slotBehaviorConfigChanged();
    void slotConfNotifications();
    void slotConfigChoqok();
    void settingsChanged();
    void slotQuit();
    void showBlog();
    void slotUpdateUnreadCount( int change, int sum );
    void slotCurrentBlogChanged(int);

    //Using this for splash screen
    void oneMicroblogLoaded();
    
Q_SIGNALS:
    void quickPostCreated();

private:
    void setupActions();
    void createQuickPostDialog();
    void disableApp();
    void enableApp();
    virtual bool queryExit();
    virtual bool queryClose();

    int mPrevUpdateInterval;
    SysTrayIcon *sysIcon;
    Choqok::UI::QuickPost *quickWidget;
    KSettings::Dialog *s_settingsDialog;
    KSplashScreen *m_splash;
    KAction *showMain;

    int microblogCounter;
};

#endif
