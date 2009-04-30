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
#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <QMap>
#include "ui_timelinewidget_base.h"
#include "datacontainers.h"
#include "backend.h"

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
    explicit TimeLineWidget( const Account &userAccount, QWidget* parent = 0 );

    ~TimeLineWidget();
    Account currentAccount() const;
    void setCurrentAccount( const Account &account );

    Backend* getBackend();
//     void setRemoved(bool isRemoved);

public slots:
    void settingsChanged();
    void updateTimeLines();
    void setUnreadStatusesToReadState();
    void abortPostNewStatus();
    void aboutQuit();

protected slots:
    void requestFavoritedDone( bool isError );
    void requestDestroyDone( bool isError );

    void directMessagesReceived( QList< Status >& msgList );
    void outboxMessagesReceived( QList< Status >& msgList );

    void homeTimeLinesReceived( QList<Status> &statusList );
    void replyTimeLineReceived( QList<Status> &statusList );

    void postingNewStatusDone( bool isError );
    void prepareReply( const QString &userName, uint statusId, bool dMsg );

    void requestDestroy( uint statusId );
    void reTweet( const QString &text );

    void checkNewStatusCharactersCount( int numOfChars );

    void postStatus( QString &status );

    void error( const QString &errMsg );


signals:
    void sigSetUnread( int unread );
    void notify( const QString &message, bool isPermanent = false );
    void systemNotify( const QString &title, const QString &message, const QString &iconUrl );
    void sigSetUnreadOnMainWin( int unread );
    void showMe();
    void sigSearch( int type, const QString & query = 0);
//     void sigStatusUpdated (bool isError);

protected:
    void checkUnreadStatuses( int numOfNewStatusesReciened );

private slots:
    void initObjects();
    void reloadFriendsList();
    void friendsListed( const QStringList &list );
    void txtNewStatusCleared();
    void revertCounterLabelShape();
    void attachMedia();
    void clearMedia();

private:
    void setDefaultDirection();
    void addNewStatusesToUi( QList< Status > & statusList, QBoxLayout *layoutToAddStatuses, QMap<uint, StatusWidget*> *list,
                             Backend::TimeLineType type = Backend::HomeTimeLine );
    void disableApp();
    void enableApp();

    /**
    * Will store current first page of statuses on disk.
    * @param fileName list will be stored in this file.
    * @param list list of statuses will be stored.
    * @return True on success, and false on fail
      */
    bool saveStatuses( QString fileName, QList< StatusWidget* > list );

    QList< Status > loadStatuses( QString fileName );

    void updateStatusList( QMap<uint, StatusWidget*> *list );

    void clearTimeLineList( QMap<uint, StatusWidget*> *list );

    void loadConfigurations();
    void updateUi();
    void showNotify( const QString &title, const QString &message );

private:
    Backend *twitter;
    StatusTextEdit *txtNewStatus;
    QLabel *lblCounter;
    QMap<uint, StatusWidget*> listHomeStatus;
    QMap<uint, StatusWidget*> listReplyStatus;
    QMap<uint, StatusWidget*> listInboxStatus;
    QMap<uint, StatusWidget*> listOutboxStatus;
    QList<StatusWidget*> listUnreadStatuses;
    uint replyToStatusId;
    bool isStartMode;//used for Notify, if true: notify will not send for any or all new twits, if false will send.

    int unreadStatusCount;
    short unreadStatusInHome;
    short unreadStatusInReply;
    short unreadStatusInOutbox;
    short unreadStatusInInbox;

    StatusWidget *toBeDestroied;

    Account mCurrentAccount;
    uint latestHomeStatusId;
    uint latestReplyStatusId;
    uint latestInboxStatusId;
    uint latestOutboxStatusId;

    QStringList friendsList;
    KUrl mediaToAttach;
};

#endif
