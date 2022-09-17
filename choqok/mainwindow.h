/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
class KCMultiDialog;

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
    KCMultiDialog *s_settingsDialog;
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
