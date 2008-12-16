//
// C++ Interface: mediamanagement
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

	QString getImageLocalPathDownloadIfNotExist(const QString &username, const QString &remotePath, QWidget *window=0);
	QString getImageLocalPathIfExist(const QString &remotePath);
	
signals:
	void sigError(QString &errMsg);
	
private:
	KConfig *mediaResource;
	KConfigGroup *map;
};

#endif
