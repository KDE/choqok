//
// C++ Implementation: mediamanagement
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mediamanagement.h"
#include <kio/netaccess.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kdebug.h>

MediaManagement::MediaManagement(QObject* parent): QObject(parent)
{
	kDebug();
	mediaResource = new KConfig();
	map = new KConfigGroup(mediaResource, "MediaMap");
}


MediaManagement::~MediaManagement()
{
	kDebug();
	map->sync();
	delete map;
	delete mediaResource;
}

QString MediaManagement::getImageLocalPathDownloadIfNotExist(const QString &username, const QString & remotePath, QWidget *window)
{
// 	kDebug();
	QString path = map->readEntry(remotePath, QString());
	if(path.isEmpty()){
// 		QString mediaDir = MEDIA_DIR;
		path = DATA_DIR + "/" + username;
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
	QString path = map->readEntry(remotePath, QString(" "));
	return path;
}

#include "mediamanagement.moc"
