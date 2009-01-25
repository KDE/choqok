/*
    This file is part of choqoK, the KDE Twitter client

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
#ifndef BACKEND_H
#define BACKEND_H
#include <QStringList>
#include <QtCore/QObject>
#include <QMap>
#include <KUrl>
#include "datacontainers.h"
#include "account.h"
class KJob;
namespace KIO{
	class Job;
}
/**
	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class Backend : public QObject
{
	Q_OBJECT
public:
	enum TimeLineType{HomeTimeLine=1, ReplyTimeLine, InboxTimeLine, OutboxTimeLine, UserTimeLine};
    enum DMessageType{Inbox=1, Outbox};
    explicit Backend(Account *account, QObject* parent=0);

    ~Backend();
	
	void login();
	void logout();
	
    void verifyCredential();
    
	QDateTime dateFromString(const QString &date);
	QString& latestErrorString();
	void quitting();
	
public slots:
	void postNewStatus(const QString &statusMessage, uint replyToStatusId=0);
    void sendDMessage( const QString &screenName, const QString &message);
	void requestTimeLine(uint latestStatusId, TimeLineType type, int page=0);
    void requestDMessages(uint latestStatusId, DMessageType type, int page=0);
	void requestFavorited(uint statusId, bool isFavorite);
	void requestDestroy(uint statusId);
    void requestDestroyDMessage(uint statusId);
	void abortPostNewStatus();
	void settingsChanged();
    void listFollowersScreenName();
    void listFriendsScreenName();

signals:
	void sigPostNewStatusDone(bool isError);
	void sigFavoritedDone(bool isError);
	void sigDestroyDone(bool isError);
	void sigError(const QString &errMsg);
	void homeTimeLineRecived(QList<Status> &statusList);
	void replyTimeLineRecived(QList<Status> &statusList);
    void userVerified(Account *userAccount);
    void directMessagesRecieved(QList<Status> &msgList);
    void outboxMessagesRecieved(QList<Status> &msgList);
    void followersListed(const QStringList &followersList);
    void friendsListed(const QStringList &friendsList);
	
protected slots:
    void slotListFollowersScreenName(KJob *job);
    void slotListFriendsScreenName(KJob *job);
	void slotPostNewStatusFinished(KJob *job);
	void slotPostNewStatusData(KIO::Job *job, const QByteArray &data);
	void slotRequestTimelineFinished(KJob *job);
	void slotRequestTimelineData(KIO::Job *job, const QByteArray &data);
	void slotRequestFavoritedFinished(KJob *job);
	void slotRequestDestroyFinished(KJob *job);
    void slotUserInfoReceived(KJob *job);
    void slotCredentialsReceived(KJob *job);
    void slotRequestDMessagesFinished(KJob *job);
	
    void slotSendDMessageFinished(KJob*);
    void slotSendDMessageData(KIO::Job*, const QByteArray&);

private:
    QStringList readUsersNameFromXml(const QByteArray &buffer);
	QList<Status> * readTimeLineFromXml(const QByteArray &buffer);
    QList<Status> * readDMessagesFromXml(const QByteArray &buffer);
	Status readStatusFromXml(const QByteArray &buffer);
    Status readDMessageFromXml(const QByteArray &buffer);
	QString prepareStatus(QString status);
    void setDefaultArgs(KUrl &url);
    void requestCurrentUser();
    void requestFollowers(int page=1);
    void requestFriends(int page=1);

	QString mLatestErrorString;
	QMap<KJob *, TimeLineType> mRequestTimelineMap;
	QMap<KJob *, QByteArray> mRequestTimelineBuffer;
    QMap<KJob *, DMessageType> mRequestDMessagesMap;

	QMap<KJob *, QByteArray> mPostNewStatusBuffer;
    QMap<KJob *, QByteArray> mSendDMessageBuffer;
    Account *mCurrentAccount;
    
    QStringList followersList;
    short followersPage;
    QStringList friendsList;
    short friendsPage;
    QString mScheme;
};

#endif
