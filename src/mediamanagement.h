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
#ifndef MEDIAMANAGEMENT_H
#define MEDIAMANAGEMENT_H

#include <QObject>

#define DATA_DIR KStandardDirs::locateLocal("data", "choqok")
#define MEDIA_DIR KStandardDirs::locateLocal("data", "choqok/media", true)

class KConfig;
class KConfigGroup;
/**
Media files management!

	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class MediaManagement : public QObject
{
	Q_OBJECT
public:
    
    
    MediaManagement(QObject* parent=0);

    ~MediaManagement();

	QString getImageLocalPathDownloadIfNotExist(const QString &username, 
			const QString &remotePath, QWidget *window=0);
	QString getImageLocalPathIfExist(const QString &remotePath);
	
signals:
	void sigError(QString &errMsg);
	
private:
	KConfig *mediaResource;
	KConfigGroup *map;
};

#endif
