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
#include "datacontainers.h"
#include "account.h"
#include "ui_prefs_base.h"
// #include "ui_accounts_base.h"
#include "ui_appears_base.h"
#include "searchwindow.h"

static const int TIMEOUT = 5000;

class QTimer;
class QuickTwit;
class SysTrayIcon;
class KTabWidget;

/**
 * This class serves as the main window for Choqok.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
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
    void updateTimeLines();
    void updateSearchResults();
    void updateSearchLimits();
//     void sigSetUnread( int unread );
    void abortPostNewStatus();
    void setUnreadStatusesToReadState();

public slots:
    void systemNotify( const QString &title, const QString &message, const QString &iconUrl );
    void search(int type = 0, const QString &search = QString());

protected slots:
    void optionsPreferences();
//     void search();
    void settingsChanged();
    void notify( const QString &message, bool isPermanent = false );
    void quitApp();
    void setNumOfUnreadOnMainWin( int unread );
    void showTimeLine();

protected:
    void hideEvent( QHideEvent * event );
    void checkUnreadStatuses( int numOfNewStatusesReciened );
    QSize sizeHint() const;

private:
    void setupActions();
    void setupQuickTweet();
    void setDefaultDirection();
    void disableApp();
    void enableApp();
    void loadConfigurations();

private slots:
    void loadAccounts();
    void addAccountTimeLine( const Account &account, bool isStartup = false );
    void removeAccountTimeLine( const QString &alias );
    void setTimeLineUpdatesEnabled( bool isEnabled );
    void setNotificationsEnabled( bool isEnabled );
    void postQuickTwit();
    void toggleMainWindow();

private:
    KTabWidget *mainWidget;
    QTimer *timelineTimer;
    Ui::prefs_base ui_prefs_base;
    Ui::appears_base ui_appears_base;
    QString currentUsername;// used for understanding username changes!
    int mPrevNotifyType;
    int mPrevUpdateInterval;
    SysTrayIcon *sysIcon;
    QuickTwit *quickWidget;
};

#endif
