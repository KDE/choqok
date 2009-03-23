/*
    This file is part of choqoK, the KDE micro-blogging client

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
#include "mediamanager.h"
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <KDE/KLocale>

MediaManager::MediaManager( QObject* parent ): QObject( parent )
{
    kDebug();
    MEDIA_DIR = KStandardDirs::locateLocal("data", "choqok/media/", true);
    mediaResource = new KConfig();
    map = new KConfigGroup( mediaResource, "MediaMap" );
}


MediaManager::~MediaManager()
{
    kDebug();
    mSelf = 0L;
    map->sync();
    delete map;
    delete mediaResource;
}

MediaManager * MediaManager::mSelf = 0L;

MediaManager * MediaManager::self()
{
    if ( !mSelf )
        mSelf = new MediaManager;
    return mSelf;
}

QString MediaManager::getImageLocalPathIfExist( const QString & remotePath )
{
    QString path = map->readEntry( remotePath, QString( ' ' ) );
    return path;
}

void MediaManager::getImageLocalPathDownloadAsyncIfNotExists( const QString & localName, const QString & remotePath )
{
//     kDebug();
    if ( mMediaFilesMap.contains( remotePath ) ) {
        ///The file is on the way, wait to download complete.
        return;
    }
    QString local;
    if ( map->hasKey( remotePath ) ) {
        local = map->readEntry( remotePath, QString() );
        emit imageFetched( remotePath, local );
    } else {
        local = MEDIA_DIR + '/' + localName;
        mMediaFilesMap [ remotePath ] = local;
        KUrl srcUrl( remotePath );
        KUrl destUrl( local );

        KIO::FileCopyJob *job = KIO::file_copy( srcUrl, destUrl, -1, KIO::HideProgressInfo | KIO::Overwrite ) ;
        if ( !job ) {
            kDebug() << "Cannot create a FileCopyJob!";
            QString errMsg = i18n( "Cannot download user image for %1, please check your Internet connection.",
                                   localName );
            emit sigError( errMsg );
            return;
        }
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotImageFetched( KJob * ) ) );
        job->start();
    }
}

void MediaManager::slotImageFetched( KJob * job )
{
//     kDebug();
    KIO::FileCopyJob *baseJob = qobject_cast<KIO::FileCopyJob *>( job );
    if ( job->error() ) {
        kDebug() << "Job error!" << job->error() << "\t" << job->errorString() <<
        "ImagePath: "<<baseJob->srcUrl().pathOrUrl();
        QString errMsg = i18n( "Cannot download user image from %1. The returned result is: %2",
                               job->errorString(), baseJob->srcUrl().pathOrUrl() );
        emit sigError( errMsg );
    } else {
        QString local = baseJob->destUrl().pathOrUrl();
        QString remote = baseJob->srcUrl().pathOrUrl();
        mMediaFilesMap.remove( remote );
        map->writeEntry( remote,  local );
        map->sync();
        emit imageFetched( remote, local );
    }
}

#include "mediamanager.moc"
