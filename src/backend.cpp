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
#include "backend.h"

#include <KDE/KLocale>
#include <QDomDocument>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kurl.h>
#include "settings.h"
#include <kio/netaccess.h>

// #define HTTP "http://twitter.com/"
// #define HTTPS "https://twitter.com/"

Backend::Backend(const Account *account, QObject* parent): QObject(parent)
{
	kDebug();
	settingsChanged();
    mCurrentAccount = account;
// 	urls[0] = ;
// 	urls[HomeTimeLine] = ;
// 	urls[ReplyTimeLine] = prefix + "statuses/replies.xml";
// 	urls[UserTimeLine] = prefix + "statuses/user_timeline.xml";
	login();	
	
}

Backend::~Backend()
{
	kDebug();
	logout();
}

void Backend::postNewStatus(const QString & statusMessage, uint replyToStatusId)
{
	kDebug();
	KUrl url(mCurrentAccount->apiPath + "/statuses/update.xml");
	url.setUser(mCurrentAccount->username);
    url.setPass(mCurrentAccount->password);
	QByteArray data = "status=";
	data += QUrl::toPercentEncoding(prepareStatus(statusMessage));
	if(replyToStatusId!=0)
		data += "&in_reply_to_status_id=" + QString::number(replyToStatusId);
	data += "&source=choqok";
	KIO::TransferJob *job = KIO::http_post(url, data, KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http POST request!";
		QString errMsg = i18n("Cannot create a http POST request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
	mPostNewStatusBuffer[ job ] = QByteArray();
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotPostNewStatusFinished(KJob*)) );
	connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)), this, SLOT(slotPostNewStatusData(KIO::Job*, const QByteArray&)));
	job->start();
}

void Backend::login()
{
	
}

void Backend::logout()
{
}

void Backend::requestTimeLine(uint latestStatusId, TimeLineType type, int page)
{
	kDebug();
	KUrl url;
	if(type==HomeTimeLine)
		url.setUrl(mCurrentAccount->apiPath + "/statuses/friends_timeline.xml");
	else
        url.setUrl(mCurrentAccount->apiPath + "/statuses/replies.xml");
	url.setUser(mCurrentAccount->username);
    url.setPass(mCurrentAccount->password);
	url.setQuery(latestStatusId ? "?since_id=" + QString::number(latestStatusId) : QString());
	kDebug()<<"Latest status Id: "<<latestStatusId;
	

	KIO::TransferJob *job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http GET request!";
		QString errMsg = i18n("Cannot create a http GET request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	mRequestTimelineMap[job] = type;
	mRequestTimelineBuffer[job] = QByteArray();
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRequestTimelineFinished(KJob*)));
	connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)), this, SLOT(slotRequestTimelineData(KIO::Job*, const QByteArray&)));
	job->start();
}

QDateTime Backend::dateFromString(const QString &date)
{
	QDateTime datetime = QDateTime::fromString(date, "ddd MMM dd h:mm:ss '+0000' yyyy");
    if(!datetime.isValid() || datetime.isNull())
        kDebug()<<"Convertion failed for \""<< date <<"\" date time, fetched from server.";
	datetime.setTimeSpec(Qt::UTC);
	return datetime.toLocalTime();
}

QList<Status> * Backend::readTimeLineFromXml(const QByteArray & buffer)
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
				kDebug()<<"there's no status tag in XML, maybe there is no new status!";
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
							} else if (node3.toElement().tagName() == "name") {
								status.user.name = node3.toElement().text();
                            } else if (node3.toElement().tagName() == "description" ) {
                                status.user.description = node3.toElement().text();
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

Status Backend::readStatusFromXml(const QByteArray & buffer)
{
	QDomDocument document;
	Status status;
	status.isError = false;
	document.setContent(buffer);
	
	QDomElement root = document.documentElement();
	
	if (root.tagName() != "status") {
		kDebug()<<"there's no status tag in XML, Error!!";
		status.isError = true;
		return status;
	}
	QDomNode node2 = root.firstChild();
	QString timeStr;
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
				} else if (node3.toElement().tagName() == "name") {
					status.user.name = node3.toElement().text();
                } else if (node3.toElement().tagName() == QString("description") ) {
                    status.user.description = node3.toElement().text();
                }
				node3 = node3.nextSibling();
			}
		}
		node2 = node2.nextSibling();
	}
	status.creationDateTime = dateFromString(timeStr);
	
	return status;
}

void Backend::abortPostNewStatus()
{
	kDebug()<<"Not implemented yet!";
// 	statusHttp.abort();
}

QString& Backend::latestErrorString()
{
	return mLatestErrorString;
}

void Backend::requestFavorited(uint statusId, bool isFavorite)
{
	kDebug();
	KUrl url;
	if(isFavorite){
        url.setUrl(mCurrentAccount->apiPath + "/favorites/create/"+QString::number(statusId)+".xml");
	
	} else {
        url.setUrl(mCurrentAccount->apiPath + "/favorites/destroy/"+QString::number(statusId)+".xml");
	}
    url.setUser(mCurrentAccount->username);
    url.setPass(mCurrentAccount->password);
	
	KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http POST request!";
		QString errMsg = i18n("Cannot create a http POST request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRequestFavoritedFinished(KJob*)) );
	
	job->start();
}

void Backend::requestDestroy(uint statusId)
{
	kDebug();
    KUrl url(mCurrentAccount->apiPath + "/statuses/destroy/"+QString::number(statusId)+".xml");
	
    url.setUser(mCurrentAccount->username);
    url.setPass(mCurrentAccount->password);
	
	KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http POST request!";
		QString errMsg = i18n("Cannot create a http POST request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRequestDestroyFinished(KJob*)) );
	
	job->start();
}

void Backend::quitting()
{
}

void Backend::slotPostNewStatusFinished(KJob * job)
{
	kDebug();
	if(job->error()){
		kDebug()<<"Error: "<<job->errorString();
		mLatestErrorString = job->errorString();
		emit sigPostNewStatusDone(true);
	} else {
// 		kDebug()<<mPostNewStatusBuffer[job];
		Status st = readStatusFromXml(mPostNewStatusBuffer[job]);
		if(st.isError){
			kDebug()<<"Error: "<<job->errorString();
			mLatestErrorString = job->errorString();
			emit sigPostNewStatusDone(true);
		} else {
			QList<Status> newSt;
			newSt.append(st);
			emit sigPostNewStatusDone(false);
			emit homeTimeLineRecived(newSt);
		}
	}
}

void Backend::slotRequestTimelineFinished(KJob *job)
{
	kDebug();
	if(!job){
		kDebug()<<"Job is null pointer";
		return;
	}
	if(job->error()){
		kDebug()<<"Error: "<<job->errorString();
		mLatestErrorString = job->errorString();
		kDebug()<<mLatestErrorString;
		emit sigError(mLatestErrorString);
		return;
	}
	QList<Status> *ptr = readTimeLineFromXml(mRequestTimelineBuffer[ job ].data());
	switch(mRequestTimelineMap.value(job)){
	case HomeTimeLine:
		if(ptr){
			emit homeTimeLineRecived(*ptr);
		} else {
			kDebug()<<"Null returned from Backend::readTimeLineFromXml()";
		}
		break;
	case ReplyTimeLine:
		if(ptr)
			emit replyTimeLineRecived(*ptr);
		else
			kDebug()<<"Null returned from Backend::readTimeLineFromXml()";
		break;
	default:
		kDebug()<<"The returned job isn't in Map!";
		break;
	};
	mRequestTimelineMap.remove(job);
	mRequestTimelineBuffer.remove(job);
}

void Backend::slotRequestTimelineData(KIO::Job * job, const QByteArray & data)
{
	kDebug();
	if( !job ) {
		kError() << "Job is a null pointer.";
		return;
	}
	unsigned int oldSize = mRequestTimelineBuffer[ job ].size();
	mRequestTimelineBuffer[ job ].resize( oldSize + data.size() );
	memcpy( mRequestTimelineBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void Backend::slotRequestFavoritedFinished(KJob * job)
{
	kDebug();
	if(!job){
		kDebug()<<"Job is null pointer.";
		return;
	}
	if(job->error()){
		kDebug()<<"Error: "<<job->errorString();
		mLatestErrorString = job->errorString();
			emit sigFavoritedDone(true);
			return;
		} else
			emit sigFavoritedDone(false);
}

void Backend::slotRequestDestroyFinished(KJob * job)
{
	kDebug();
	if(!job){
		kDebug()<<"Job is null pointer.";
		return;
	}
	if(job->error()){
		kDebug()<<"Error: "<<job->errorString();
		mLatestErrorString = job->errorString();
		emit sigDestroyDone(true);
		return;
	} else
		emit sigDestroyDone(false);
}

QString Backend::prepareStatus(QString status)
{
	kDebug();
	QString t="";
	int i = 0, j = 0;
	while ((j = status.indexOf("http://", i)) != -1) {
		t += status.mid(i, j - i);
		int k = status.indexOf(' ', j);
		if (k == -1) k = status.length();
		QString baseUrl = status.mid(j, k - j);
		if(baseUrl.count()>30){
			KUrl url("http://is.gd/api.php");
			url.addQueryItem("longurl", baseUrl) ;
			
			KIO::Job *job = KIO::get( url, KIO::Reload, KIO::HideProgressInfo );
			QMap<QString, QString> metaData;
			QByteArray data;
			metaData.insert( "PropagateHttpHeader", "true" );
			if ( KIO::NetAccess::synchronousRun( job, 0, &data, 0, &metaData ) ) {
				QString responseHeaders = metaData[ "HTTP-Headers" ];
				QString code = responseHeaders.split(' ')[1];
				if(code=="200"){
					kDebug()<<"Short url is: "<< data;
					t += QString(data);
				} else {
					kDebug()<<"shortenning url faild HTTP response code is: "<<code;
					t += baseUrl;
				}
			} else {
				QString responseHeaders = metaData[ "HTTP-Headers" ];
				kDebug()<<"Cannot create a shorten url.\t"<<"Response header = "<< responseHeaders;
				t += baseUrl;
			}
		} else {
			t += baseUrl;
		}
		i = k;
	}
	t += status.mid(i);
	return t;
}

void Backend::settingsChanged()
{
// 	if(Settings::useSecureConnection())
// 		prefix = mCurrentAccount->apiPath;
// 	else
// 		prefix = HTTP;
}

void Backend::slotPostNewStatusData(KIO::Job * job, const QByteArray & data)
{
	kDebug();
	if( !job ) {
		kError() << "Job is a null pointer.";
		return;
	}
	unsigned int oldSize = mPostNewStatusBuffer[ job ].size();
	mPostNewStatusBuffer[ job ].resize( oldSize + data.size() );
	memcpy( mPostNewStatusBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

#include "backend.moc"
