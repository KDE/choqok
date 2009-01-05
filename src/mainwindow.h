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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kxmlguiwindow.h>
#include "datacontainers.h"
#include "ui_prefs_base.h"
// #include "ui_accounts_base.h"
#include "ui_appears_base.h"

#define TIMEOUT 5000

class QTimer;

class KTabWidget;

/**
 * This class serves as the main window for choqoK.  It handles the
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
    void systemNotify(const QString &title, const QString &message, const QString &iconUrl);
    void updateTimeLines();
    void sigSetUnread ( int unread );
    void abortPostNewStatus();
    void setUnreadStatusesToReadState();
    void accountAdded(const Account &account);
    void accountRemoved(const QString &alias);

public slots:
//     void setUnreadStatusesToReadState();

protected slots:
    void optionsPreferences();
    void settingsChanged();
    void notify ( const QString &message );
    void quitApp();

protected:
    void keyPressEvent ( QKeyEvent * e );
    void checkUnreadStatuses ( int numOfNewStatusesReciened );
    bool queryClose();

private:
    void setupActions();
    void setDefaultDirection();
    void disableApp();
    void enableApp();
    void loadConfigurations();
//     void addNewAccountTimeLine(const Account &account);
    
private slots:
    void loadAccounts();
    void addAccountTimeLine(const Account &account);
    void removeAccountTimeLine(const QString &alias);
//     void updateAccountTimeLine(const Account &account);

private:
    KTabWidget *mainWidget;
    QTimer *timelineTimer;
    Ui::prefs_base ui_prefs_base;
//     Ui::accounts_base ui_accounts_base;
    Ui::appears_base ui_appears_base;
    QString currentUsername;// used for undresanding of username changes!
};

#endif
