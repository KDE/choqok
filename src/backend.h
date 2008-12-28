//
// C++ Interface: backend
//
// Description: the Backend of choqoK
//
//
// Author:  Mehrdad Momeny (C) 2008
// Email Address: mehrdad.momeny[AT]gmail.com
// Copyright: GNU GPL v3
//
//
#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore/QObject>
#include <QMap>
#include "datacontainers.h"
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
    Backend(QObject* parent=0);

    ~Backend();
	
	void login();
	void logout();
	
	QDateTime dateFromString(const QString &date);
	QString& latestErrorString();
	void quiting();
	
public slots:
	void postNewStatus(const QString &statusMessage, uint replyToStatusId=0);
	void requestTimeLine(TimeLineType type, int page=0);
	void requestFavorited(uint statusId, bool isFavorite);
	void requestDestroy(uint statusId);
	void abortPostNewStatus();
// 	void requestCurrentUser();
	
signals:
	void sigPostNewStatusDone(bool isError);
	void sigFavoritedDone(bool isError);
	void sigDestroyDone(bool isError);
	void sigError(QString &errMsg);
	void homeTimeLineRecived(QList<Status> &statusList);
	void replyTimeLineRecived(QList<Status> &statusList);
	void currentUserInfo(User);
// 	void directMessagesRecived(QList<Status> &statusList);
	
protected slots:
	void slotPostNewStatusFinished(KJob *job);
	void slotRequestTimelineFinished(KJob *job);
	void slotRequestTimelineData(KIO::Job *job, const QByteArray &data);
	void slotRequestFavoritedFinished(KJob *job);
	void slotRequestDestroyFinished(KJob *job);
	
private:
	QList<Status> * readTimeLineFromXml(const QByteArray &buffer);
	QString urls[4];
	
	QString mLatestErrorString;
	QMap<KJob *, TimeLineType> mRequestTimelineMap;
	QMap<KJob *, QByteArray> mRequestTimelineBuffer;
};

#endif
