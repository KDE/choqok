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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHideEvent>
#include <QPointer>
#include <QShowEvent>

#include "choqokmainwindow.h"
#include "account.h"
#include "plugin.h"

class QAction;
class QPushButton;
class QSplashScreen;
class ChoqokApplication;
namespace Choqok
{
namespace UI
{
class QuickPost;
}
}
namespace KSettings
{
class Dialog;
}

class SysTrayIcon;

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
    MainWindow(ChoqokApplication *application);

    /**
    * Default Destructor
    */
    virtual ~MainWindow();

protected:
    virtual void hideEvent(QHideEvent *event) override;
    virtual void showEvent(QShowEvent *) override;

private Q_SLOTS:
    void nextTab(int delta, Qt::Orientation orientation);
    void loadAllAccounts();
    void newPluginAvailable(Choqok::Plugin *plugin);
    void addBlog(Choqok::Account *account, bool isStartup = false);
    void updateBlog(Choqok::Account *account, bool enabled);
    void removeBlog(const QString &alias);
    void setTimeLineUpdatesEnabled(bool isEnabled);
    void setNotificationsEnabled(bool isEnabled);
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
    void slotUpdateUnreadCount(int change, int sum);
    void slotCurrentBlogChanged(int);

    //Using this for splash screen
    void oneMicroblogLoaded();
    void slotShowSpecialMenu(bool show);
    void slotDonate();

private:
    void updateTabbarHiddenState();
    void setupActions();
    void createQuickPostDialog();
    void disableApp();
    void enableApp();
    void updateSysTray();

    int mPrevUpdateInterval;
    SysTrayIcon *sysIcon;
    Choqok::UI::QuickPost *quickWidget;
    KSettings::Dialog *s_settingsDialog;
    QPointer<QSplashScreen> m_splash;
    QAction *enableUpdates;
    QAction *newTwit;
    QAction *showMain;
    QAction *actQuit;
    QAction *actUpdate;
    QAction *prefs;
    QAction *aboutChoqok;
    QPushButton *choqokMainButton;
    ChoqokApplication *app;

    int microblogCounter;
    bool choqokMainButtonVisible;
};

#endif
