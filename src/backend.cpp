//
// C++ Implementation: backend
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "backend.h"

#include <QHttp>
#include <QHttpRequestHeader>
#include <KDE/KLocale>
#include <QDomDocument>

#include "settings.h"

Backend::Backend(QObject* parent): QObject(parent)
{
	kDebug();
	urls[HomeTimeLine] = "http://twitter.com/statuses/friends_timeline.xml";
	urls[ReplyTimeLine] = "http://twitter.com/statuses/replies.xml";
	urls[UserTimeLine] = "http://twitter.com/statuses/user_timeline.xml";
	login();	
	
	connect(&statusHttp, SIGNAL(requestFinished( int, bool )), this, SLOT(postNewStatusFinished(int, bool)));
	connect(&timelineHttp, SIGNAL(requestFinished( int, bool )), this, SLOT(requestTimelineFinished(int, bool)));
}

Backend::~Backend()
{
	kDebug();
	logout();
// 	quiting();
}

void Backend::postNewStatus(const QString & statusMessage, uint replyToStatusId)
{
	kDebug();
	QUrl url("http://twitter.com/statuses/update.xml");
	QHttpRequestHeader header;
	header.setRequest("POST", url.path());
	header.setValue("Host", url.host());
	header.setContentType("application/x-www-form-urlencoded");
// 	header.setValue("X-Twitter-Client", "choqoK");
// 	header.setValue("X-Twitter-Client-Version", "0.1");
// 	header.setValue("X-Twitter-Client-URL", "http://github.com/mtux/choqok/wikis/home");
	
 	statusHttp.setHost(url.host(), url.port(80));
	statusHttp.setUser(Settings::username(), Settings::password());
	
	QByteArray data = "status=";
	data += QUrl::toPercentEncoding(statusMessage);
	if(replyToStatusId!=0)
		data += "&in_reply_to_status_id=" + QString::number(replyToStatusId);
	data += "&source=choqok";
	
	statusHttpNum = statusHttp.request(header, data);
}

void Backend::login()
{
	
}

void Backend::logout()
{
}

void Backend::requestTimeLine(TimeLineType type, int page)
{
	kDebug();
	QUrl url(urls[type]);
	timelineHttp.setHost(url.host(), url.port(80));
	timelineHttp.setUser(Settings::username(), Settings::password());
	QString path = url.toString() + (Settings::latestStatusId() ? "?since_id=" + QString::number(Settings::latestStatusId()) : "");
	kDebug()<<"Latest status Id: "<<Settings::latestStatusId();
	switch(type){
		case HomeTimeLine:
// 			connect(&timelineHttp, SIGNAL(done( bool )), this, SLOT(homeTimeLineDone(bool)));
			homeBuffer.open(QIODevice::WriteOnly);
			homeTimelineHttpNum = timelineHttp.get( path, &homeBuffer);
			break;
		case ReplyTimeLine:
// 			connect(&timelineHttp, SIGNAL(done( bool )), this, SLOT(replyTimeLineDone(bool)));
			replyBuffer.open(QIODevice::WriteOnly);
			replyTimelineHttpNum = timelineHttp.get( path, &replyBuffer);
			break;
		default:
			kDebug()<<"Unknown TimeLine Type";
			break;
	};
}

void Backend::requestTimelineFinished(int id, bool isError)
{
	if(isError){
		mLatestErrorString = getErrorString(qobject_cast<QHttp *>(sender()));
		kDebug()<<mLatestErrorString;
		return;
	}
	QByteArray tmp;
	if(id == homeTimelineHttpNum){
		kDebug();
		tmp = homeBuffer.data();
		homeBuffer.close();
		
		QList<Status> *ptr = readTimeLineFromXml(tmp);
		if(ptr)
			emit homeTimeLineRecived(*ptr);
		else
			kDebug()<<"Null returned from Backend::readTimeLineFromXml()";
	} else if (id == replyTimelineHttpNum){
		kDebug();
		tmp = replyBuffer.data();
		homeBuffer.close();
		
		QList<Status> *ptr = readTimeLineFromXml(tmp);
		if(ptr)
			emit replyTimeLineRecived(*ptr);
		else
			kDebug()<<"Null returned from Backend::readTimeLineFromXml()";
	}
}

QString Backend::getErrorString(QHttp * sender)
{
	kDebug();
	QString errType;
	switch(sender->error()){
		case QHttp::NoError:
			errType = i18n("No error occurred! ");
			break;
		case QHttp::HostNotFound:
// 			errType = i18n("Host not found, ");
			break;
		case QHttp::ConnectionRefused:
			errType = i18n("Connection refused by server, please try again later. ");
			break;
		case QHttp::UnexpectedClose:
			errType = i18n("Connection terminated unexpectedly, please try again later. ");
			break;
		case QHttp::Aborted:
// 			errType = i18n("Aborted, ");
			break;
		case QHttp::InvalidResponseHeader:
			errType = i18n("Invalid response header, ");
			break;
		case QHttp::WrongContentLength:
			errType = i18n("Wrong Content length, ");
			break;
		case QHttp::UnknownError:
		default:
// 			errType = i18n("Unknown error occured, ");
			break;
	}
	errType += sender->errorString();
	
	return errType;
}

QDateTime Backend::dateFromString(const QString &date)
{
	QDateTime datetime = QDateTime::fromString(date, "ddd MMM dd hh:mm:ss '+0000' yyyy");
	datetime.setTimeSpec(Qt::UTC);
	return datetime.toLocalTime();
}

QList<Status> * Backend::readTimeLineFromXml(QByteArray & buffer)
{
	kDebug();
	QDomDocument document;
	QList<Status> *statusList = new QList<Status>;
	
	document.setContent(buffer);
	
	QDomElement root = document.documentElement();
	
	if (root.tagName() != "statuses") {
		QString err = i18n("Data returned from server corrupted!");
		kDebug()<<"there's no statuses tag in XML\t the XML is: \n"<<buffer.data();
		mLatestErrorString = err;
		return 0;
	}
		QDomNode node = root.firstChild();
		QString timeStr;
		while (!node.isNull()) {
			if (node.toElement().tagName() != "status") {
				kDebug()<<"there no status tag in XML, maybe there is no new status!";
				return statusList;
			}
				QDomNode node2 = node.firstChild();
				Status status;
				while (!node2.isNull()) {
					if(node2.toElement().tagName() == "created_at")
						timeStr = node2.toElement().text();
					else if(node2.toElement().tagName() == "text")
						status.content = node2.toElement().text();
					else if(node2.toElement().tagName() == "id")
						status.statusId = node2.toElement().text().toInt();
					else if(node2.toElement().tagName() == "in_reply_to_status_id")
						status.replyToStatusId = node2.toElement().text().toULongLong();
					else if(node2.toElement().tagName() == "in_reply_to_user_id")
						status.replyToUserId = node2.toElement().text().toULongLong();
					else if(node2.toElement().tagName() == "in_reply_to_screen_name")
						status.replyToUserScreenName = node2.toElement().text();
					else if(node2.toElement().tagName() == "source")
						status.source = node2.toElement().text();
					else if(node2.toElement().tagName() == "truncated")
						status.isTruncated = (node2.toElement().text() == "true")? true : false;
					else if(node2.toElement().tagName() == "favorited")
						status.isFavorited = (node2.toElement().text() == "true")? true : false;
					else if(node2.toElement().tagName() == "user"){
						QDomNode node3 = node2.firstChild();
						while (!node3.isNull()) {
							if (node3.toElement().tagName() == "screen_name") {
								status.user.screenName = node3.toElement().text();
							} else if (node3.toElement().tagName() == "profile_image_url") {
								status.user.profileImageUrl = node3.toElement().text();
							} else if (node3.toElement().tagName() == "id") {
								status.user.userId = node3.toElement().text().toUInt();
							}
							node3 = node3.nextSibling();
						}
					}
					node2 = node2.nextSibling();
				}
				node = node.nextSibling();
				status.creationDateTime = dateFromString(timeStr);
// 				 = QDateTime(time.date(), time.time(), Qt::UTC);
				statusList->insert(0, status);
			}
	return statusList;
}

void Backend::abortPostNewStatus()
{
	statusHttp.abort();
}

QString& Backend::latestErrorString()
{
	return mLatestErrorString;
}

void Backend::postNewStatusFinished(int id, bool isError)
{
	if(id == statusHttpNum){
		kDebug();
		if(isError){
			QString err = getErrorString(qobject_cast<QHttp *>(sender()));
			kDebug()<<err;
			mLatestErrorString = err;
			emit sigPostNewStatusDone(true);
			return;
		} else
			emit sigPostNewStatusDone(false);
	} else if ( id == favoritedHttpNum){
		kDebug();
		if(isError){
			QString err = getErrorString(qobject_cast<QHttp *>(sender()));
			kDebug()<<err;
			mLatestErrorString = err;
			emit sigFavoritedDone(true);
			return;
		} else
			emit sigFavoritedDone(false);
	} else if ( id==destroyHttpNum){
		kDebug();
		if(isError){
			QString err = getErrorString(qobject_cast<QHttp *>(sender()));
			kDebug()<<err;
			mLatestErrorString = err;
			emit sigDestroyDone(true);
			return;
		} else
			emit sigDestroyDone(false);
	}
}

void Backend::requestFavorited(uint statusId, bool isFavorite)
{
	kDebug();
	if(isFavorite){
		QUrl url("http://twitter.com/favorites/create/"+QString::number(statusId)+".xml");
	
		statusHttp.setHost(url.host(), url.port(80));
		statusHttp.setUser(Settings::username(), Settings::password());
	
		QByteArray data = "source=choqok";
	
		favoritedHttpNum = statusHttp.post(url.toString() , data);
	} else {
		QUrl url("http://twitter.com/favorites/destroy/"+QString::number(statusId)+".xml");
	
		statusHttp.setHost(url.host(), url.port(80));
		statusHttp.setUser(Settings::username(), Settings::password());
	
		QByteArray data = "source=choqok";
	
		favoritedHttpNum = statusHttp.post(url.toString() , data);
	}
}

void Backend::requestDestroy(uint statusId)
{
	kDebug();
	QUrl url("http://twitter.com/statuses/destroy/"+QString::number(statusId)+".xml");
	
	statusHttp.setHost(url.host(), url.port(80));
	statusHttp.setUser(Settings::username(), Settings::password());
	
	QByteArray data = "source=choqok";
	
	destroyHttpNum = statusHttp.post(url.toString() , data);
}

void Backend::quiting()
{
// 	statusHttp.abort();
	statusHttp.close();
	
// 	timelineHttp.abort();
	timelineHttp.close();
}


#include "backend.moc"
