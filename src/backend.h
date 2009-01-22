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
#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore/QObject>
#include <QMap>
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
	enum TimeLineType{HomeTimeLine=1, ReplyTimeLine, UserTimeLine};
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
	void requestTimeLine(uint latestStatusId, TimeLineType type, int page=0);
	void requestFavorited(uint statusId, bool isFavorite);
	void requestDestroy(uint statusId);
	void abortPostNewStatus();
	void settingsChanged();
	
signals:
	void sigPostNewStatusDone(bool isError);
	void sigFavoritedDone(bool isError);
	void sigDestroyDone(bool isError);
	void sigError(QString &errMsg);
	void homeTimeLineRecived(QList<Status> &statusList);
	void replyTimeLineRecived(QList<Status> &statusList);
    void userVerified(Account *userAccount);
// 	void directMessagesRecived(QList<Status> &statusList);
	
protected slots:
	void slotPostNewStatusFinished(KJob *job);
	void slotPostNewStatusData(KIO::Job *job, const QByteArray &data);
	void slotRequestTimelineFinished(KJob *job);
	void slotRequestTimelineData(KIO::Job *job, const QByteArray &data);
	void slotRequestFavoritedFinished(KJob *job);
	void slotRequestDestroyFinished(KJob *job);
    void slotUserInfoReceived(KJob *job);
    void slotCredentialsReceived(KJob *job);
	
private:
	QList<Status> * readTimeLineFromXml(const QByteArray &buffer);
	Status readStatusFromXml(const QByteArray &buffer);
	QString prepareStatus(QString status);
    void requestCurrentUser();
	
// 	QString urls[4];
// 	QString prefix;
	QString mLatestErrorString;
	QMap<KJob *, TimeLineType> mRequestTimelineMap;
	QMap<KJob *, QByteArray> mRequestTimelineBuffer;
	

	QMap<KJob *, QByteArray> mPostNewStatusBuffer;
    
    Account *mCurrentAccount;
//     int latestStatusId;
};

#endif
