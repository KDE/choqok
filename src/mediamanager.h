/*
    This file is part of choqoK, the KDE mono-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/
#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <QObject>
#include <QMap>
#define DATA_DIR KStandardDirs::locateLocal("data", "choqok/")
#define MEDIA_DIR KStandardDirs::locateLocal("data", "choqok/media/", true)
class QPixmap;
class KConfig;
class KConfigGroup;
namespace KIO
{
    class Job;
}
class KJob;
/**
Media files manager!

    @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class MediaManager : public QObject
{
    Q_OBJECT
public:


    MediaManager( QObject* parent = 0 );

    ~MediaManager();

    static MediaManager *self();

    QString getImageLocalPathIfExist( const QString &remotePath );

    void getImageLocalPathDownloadAsyncIfNotExists( const QString &localName, const QString &remotePath );

signals:
    void sigError( QString &errMsg );
    void imageFetched( const QString &remotePath, const QString &localPath );

protected slots:
    void slotImageFetched( KJob *job );

private:
    static MediaManager *mSelf;
    KConfig *mediaResource;
    KConfigGroup *map;
    QMap<QString, QString> mMediaFilesMap;// QMap<RemotePath, LocalPath>
};

#endif
