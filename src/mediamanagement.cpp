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
#include "mediamanagement.h"
#include <QPixmap>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <KDE/KLocale>

MediaManagement::MediaManagement(QObject* parent): QObject(parent)
{
// 	kDebug();
	mediaResource = new KConfig();
	map = new KConfigGroup(mediaResource, "MediaMap");
}


MediaManagement::~MediaManagement()
{
// 	kDebug();
	map->sync();
	delete map;
	delete mediaResource;
}

QString MediaManagement::getImageLocalPathDownloadIfNotExist(const QString &username, 
		const QString & remotePath, QWidget *window)
{
// 	kDebug();
	QString path = map->readEntry(remotePath, QString());
	if(path.isEmpty()){
		path = MEDIA_DIR + '/' + username;
		if(KIO::NetAccess::download(remotePath, path, window)){
			map->writeEntry(remotePath, path);
			return path;
		} else{
			QString err = KIO::NetAccess::lastErrorString();
			emit sigError(err);
			return QString();
		}
	} else {
		return path;
	}
}

QString MediaManagement::getImageLocalPathIfExist(const QString & remotePath)
{
	KConfig conf;
	KConfigGroup stMap(&conf, "MediaMap");
	QString path = stMap.readEntry(remotePath, QString(" "));
	return path;
}

void MediaManagement::getImageLocalPathDownloadAsyncIfNotExists(const QString & username, const QString & remotePath)
{
	local = map->readEntry(remotePath, QString());
	if(local.isEmpty()){
		remote = remotePath;
		KUrl srcUrl(remotePath);
		local = MEDIA_DIR+'/'+username;
		KUrl destUrl(local);
		
		KIO::FileCopyJob *job = KIO::file_copy(srcUrl, destUrl, -1, KIO::HideProgressInfo | KIO::Overwrite) ;
		if(!job){
			kDebug()<<"Cannot create a FileCopyJob!";
			QString errMsg = i18n("Cannot download userimage for %1, please check your internet connection.", username);
			emit sigError(errMsg);
			return;
		}
		connect( job, SIGNAL(result(KJob*)), this, SLOT(imageFetched(KJob *)));
		job->start();
	} else {
		emit imageLocalPath(local);
	}
}

void MediaManagement::imageFetched(KJob * job)
{
	if(job->error()){
		kDebug()<<"Job error!"<<job->error()<<"\t"<<job->errorString();
		QString errMsg = i18n("Cannot download userimage, The returned result is: %1.", job->errorText());
		emit sigError(errMsg);
	} else {
		map->writeEntry(remote,  local);
		map->sync();
		emit imageLocalPath(local);
	}
}

#include "mediamanagement.moc"
