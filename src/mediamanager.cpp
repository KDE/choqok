/*
    This file is part of Choqok, the KDE micro-blogging client

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
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kdebug.h>
#include <KDE/KLocale>
#include <kicon.h>

MediaManager::MediaManager( QObject* parent ): QObject( parent ),mEmoticons(KEmoticons().theme()),mCache("choqok-userimages")
{
  KIcon icon("image-loading");
  mDefaultImage = icon.pixmap(48);
  mCache.setCacheLimit(20000);
}

MediaManager::~MediaManager()
{
    mSelf = 0L;
    kDebug();
}

MediaManager * MediaManager::mSelf = 0L;

MediaManager * MediaManager::self()
{
    if ( !mSelf )
        mSelf = new MediaManager;
    return mSelf;
}

QString MediaManager::parseEmoticons(const QString& text)
{
  return mEmoticons.parseEmoticons(text,KEmoticonsTheme::DefaultParse,QStringList() << "(e)");
}

QPixmap * MediaManager::getAvatarIfExist( const KUrl & remotePath )
{
    QPixmap *p = new QPixmap();
    if(!mCache.find(remotePath.url(),*p))
      return 0;
    return p;
}

void MediaManager::getAvatarDownloadAsyncIfNotExist( const QString & remotePath )
{
    KUrl srcUrl( remotePath );
    QString url = srcUrl.url(KUrl::RemoveTrailingSlash);
    if ( mQueue.contains( url ) ) {
        ///The file is on the way, wait to download complete.
        return;
    }
    QPixmap p;
    if ( mCache.find( url, p ) ) {
        emit avatarFetched( url, p );
    } else {
        KIO::Job *job = KIO::storedGet( srcUrl, KIO::NoReload, KIO::HideProgressInfo ) ;
        if ( !job ) {
            kDebug() << "Cannot create a FileCopyJob!";
            QString errMsg = i18n( "Cannot download user image, please check your Internet connection.");
            emit avatarFetchError( url, errMsg );
            return;
        }
        mQueue.insert( url );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotImageFetched( KJob * ) ) );
        job->start();
    }
}

void MediaManager::slotImageFetched( KJob * job )
{
    KIO::StoredTransferJob *baseJob = qobject_cast<KIO::StoredTransferJob *>( job );
    QString remote = baseJob->url().url(KUrl::RemoveTrailingSlash);
    mQueue.remove( remote );
    if ( job->error() ) {
        kDebug() << "Job error!" << job->error() << "\t" << job->errorString();
        QString errMsg = i18n( "Cannot download user image from %1.",
                               job->errorString() );
        emit avatarFetchError( remote, errMsg );
//         emit avatarFetched(remote, KIcon("image-missing").pixmap(48));
    } else {
        QPixmap p;
        if( p.loadFromData( baseJob->data() ) ) {
            mCache.insert( remote, p );
            emit avatarFetched( remote, p );
        } else {
            emit avatarFetchError( remote,"download failed" );
        }
    }
}

void MediaManager::clearAvatarCache()
{
  mCache.discard();
}

#include "mediamanager.moc"
