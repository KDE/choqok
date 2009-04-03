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
#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <QObject>
#include <QSet>

#include <KEmoticons>
#include <KEmoticonsTheme>

#include <KPixmapCache>

class QPixmap;
class KUrl;

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

    QPixmap * getImageLocalPathIfExist( const KUrl& remotePath );
    void getImageLocalPathDownloadAsyncIfNotExists( const QString & value, const QString &remotePath );
    QString parseEmoticons(const QString & text);

signals:
    void sigError( QString &errMsg );
    void imageFetched( const QString & url, const QPixmap & pixmap );

protected slots:
    void slotImageFetched( KJob *job );

private:
    static MediaManager * mSelf;
    KEmoticonsTheme mEmoticons;
    KPixmapCache mCache;
    QMap<QString,QString> mQueue;
};

#endif
