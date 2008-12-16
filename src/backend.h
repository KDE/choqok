//
// C++ Interface: backend
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore/QObject>

#include "datacontainers.h"
#include "QBuffer"
#include "QHttp"
// class QNetworkAccessManager;

/**
	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class Backend : public QObject
{
	Q_OBJECT
public:
	enum TimeLineType{All=0, HomeTimeLine, ReplyTimeLine, UserTimeLine};
    Backend(QObject* parent=0);

    ~Backend();
	
	void login();
	void logout();
	
	QDateTime dateFromString(const QString &date);
	QString& latestErrorString();
	void quiting();
	
public slots:
// 	void updateTimeLines(TimeLineType type=All, int page=0);
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
// 	void sigError(QString &errorMessage);
	void homeTimeLineRecived(QList<Status> &statusList);
	void replyTimeLineRecived(QList<Status> &statusList);
	void currentUserInfo(User);
// 	void directMessagesRecived(QList<Status> &statusList);
	
protected slots:
	void requestTimelineFinished(int id, bool isError);
	void postNewStatusFinished(int id, bool isError);
	
private:
	QString getErrorString(QHttp *sender);
	QList<Status> * readTimeLineFromXml(QByteArray &buffer);
	QString urls[4];
	QBuffer homeBuffer;
	QBuffer replyBuffer;
	QBuffer userIdBuffer;
	
	QHttp statusHttp;
	int statusHttpNum;
	int favoritedHttpNum;
	int destroyHttpNum;
	
	QHttp timelineHttp;
	int homeTimelineHttpNum;
	int replyTimelineHttpNum;
	
	QMap<QString, int> monthes;
	QString mLatestErrorString;
};

#endif
