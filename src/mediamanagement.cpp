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

QString MediaManagement::getImageLocalPathDownloadIfNotExist(const QString &username, 
		const QString & remotePath, QWidget *window)
{
// 	kDebug();
	QString path = map->readEntry(remotePath, QString());
	if(path.isEmpty()){
// 		QString mediaDir = MEDIA_DIR;
		path = DATA_DIR + '/' + username;
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
