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
#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include "ui_timelinewidget_base.h"
#include "datacontainers.h"
#include "backend.h"

#define TIMEOUT 5000

class Backend;
class QLabel;
class StatusTextEdit;
class StatusWidget;

/**
The timeline + Updating status widget.
Using this for any Account/Service to make it simple supporting multiple accounts.

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/

class TimeLineWidget : public QWidget, public Ui::timelinewidget_base
{
    Q_OBJECT
public:
    TimeLineWidget ( const Account &userAccount, QWidget* parent = 0 );

    ~TimeLineWidget();
    Account currentAccount() const;
    void setCurrentAccount(const Account &account);

public slots:
    void settingsChanged();
    void updateTimeLines();
    void setUnreadStatusesToReadState();
    void abortPostNewStatus();

protected slots:
//     void toggleTwitFieldVisible();
    void requestFavoritedDone ( bool isError );
    void requestDestroyDone ( bool isError );

    void homeTimeLinesRecived ( QList<Status> &statusList );
    void replyTimeLineRecived ( QList<Status> &statusList );
    void postingNewStatusDone ( bool isError );
    void prepareReply ( QString &userName, uint statusId );

    void requestDestroy ( uint statusId );
//   void notify(const QString &message);

    void checkNewStatusCharactersCount ( int numOfChars );

    void postStatus ( QString &status );

    void error ( QString &errMsg );


signals:
    void sigSetUnread ( int unread );
    void notify ( const QString &message );
    void systemNotify ( const QString &title, const QString &message, const QString &iconUrl );
    void sigSetUnreadOnMainWin( int unread );

protected:
    void checkUnreadStatuses ( int numOfNewStatusesReciened );

private slots:
    void initObjects();

private:
    void setDefaultDirection();
    void addNewStatusesToUi ( QList< Status > & statusList, QBoxLayout *layoutToAddStatuses, QList<StatusWidget*> *list,
                              Backend::TimeLineType type = Backend::HomeTimeLine );
    void disableApp();
    void enableApp();

    /**
    * Will store current first page of statuses on disk.
    * @param fileName list will be stored on this file.
    * @param list list of Statuses will be stored.
    * @return True on success, and false on failer
      */
    bool saveStatuses ( QString fileName, QList<StatusWidget*> &list );

    QList< Status > loadStatuses ( QString fileName );

    void updateStatusList ( QList<StatusWidget*> *list );

    void reloadTimeLineLists();
    void clearTimeLineList ( QList<StatusWidget*> *list );

    void loadConfigurations();
    QString generateStatusBackupFileName(Backend::TimeLineType type);

private:
    Backend *twitter;
    StatusTextEdit *txtNewStatus;
    QLabel *lblCounter;
    QList<StatusWidget*> listHomeStatus;
    QList<StatusWidget*> listReplyStatus;
    QList<StatusWidget*> listUnreadStatuses;
    uint replyToStatusId;
//   QString currentUsername;// used for undresanding of username changes!
    bool isStartMode;//used for Notify, if true: notify will not send for any or all new twits, if false will send.

    int unreadStatusCount;
    short unreadStatusInHome;
    short unreadStatusInReply;

    StatusWidget *toBeDestroied;

    Account mCurrentAccount;
    uint latestStatusId;
};

#endif
