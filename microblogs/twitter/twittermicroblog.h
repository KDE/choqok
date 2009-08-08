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
#ifndef TWITTERMICROBLOGPLUGIN_H
#define TWITTERMICROBLOGPLUGIN_H

#include <microblog.h>
#include <KUrl>

class ChoqokEditAccountWidget;
class KJob;

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class TwitterMicroBlog : public Choqok::MicroBlog
{
Q_OBJECT
public:
    TwitterMicroBlog( QObject* parent, const QStringList& args  );
    ~TwitterMicroBlog();

    virtual Choqok::Account *createAccount( const QString &alias );
    virtual ChoqokEditAccountWidget * createEditAccountWidget( Choqok::Account *account, QWidget *parent );
    virtual Choqok::MicroBlogWidget * createMicroBlogWidget( Choqok::Account *account, QWidget *parent );
    virtual Choqok::TimelineWidget * createTimelineWidget( Choqok::Account* account,
                                                           const QString& timelineName, QWidget* parent );
    virtual Choqok::PostWidget* createPostWidget(Choqok::Account* account,
                                                  const Choqok::Post& post, QWidget* parent);
    virtual QString profileUrl(const QString &username) const;

    virtual QList< Choqok::Post* > loadTimeline(const QString& accountAlias, const QString& timelineName);
    virtual void saveTimeline(const QString& accountAlias, const QString& timelineName, QList< Choqok::PostWidget* > timeline);
public slots:

    /**
    \brief Create a new post

    @see postCreated()
    @see abortCreatePost()
    */
    virtual void createPost( Choqok::Post *post );

    /**
    \brief Abort all of createPost requests!
    */
    virtual void abortCreatePost();

    /**
    \brief Fetch a post

    @see postFetched()
    */
    virtual void fetchPost( Choqok::Post *post );

    /**
    \brief Remove a post

    @see postRemoved()
    */
    virtual void removePost( Choqok::Post *post );

    /**
    Request to update all timelines of account!
    They will arrive in several signals! with timelineDataReceived() signal!

    @see timelineDataReceived()
    */
    virtual void updateTimelines();

    virtual void createFavorite( const QString &postId );
    virtual void removeFavorite( const QString &postId );

signals:
    void favoriteCreated(const QString &postId);
    void favoriteRemoved(const QString &postId);
//     void errorPost( const QString &errorString, const Choqok::Post &post );

protected slots:
    virtual void requestTimeLine( QString type, QString latestStatusId, int page = 0, QString maxId = 0 );
    virtual void slotCreatePost( KJob *job );
    virtual void slotFetchPost( KJob *job );
    virtual void slotRemovePost( KJob *job );
    virtual void slotCreateFavorite( KJob *job  );
    virtual void slotRemoveFavorite( KJob *job  );
    virtual void slotRequestTimeline( KJob *job  );

protected:
    virtual void setDefaultArgs( KUrl &url );
    virtual QDateTime dateFromString( const QString &date );
    virtual Choqok::Post * readPostFromDomElement( const QDomElement &root, Choqok::Post *post=0 );
    virtual Choqok::Post * readPostFromXml( const QByteArray &buffer, Choqok::Post *post=0 );
    virtual QList<Choqok::Post*> readTimelineFromXml( const QByteArray &buffer );
    virtual Choqok::Post * readDMessageFromXml ( const QByteArray &buffer );
    virtual Choqok::Post * readDMessageFromDomElement ( const QDomElement& root );
    virtual QList<Choqok::Post*> readDMessagesFromXml ( const QByteArray &buffer );

    virtual QString postUrl ( const QString &postId, const QString &userScreenName );


    QMap<QString, int> monthes;
    QHash<QString, QString> timelineApiPath;//TimelineType, path

    QMap<KJob*, QString> mFavoriteMap;//Job, postId
    QMap<KJob*, Choqok::Post*> mRemovePostMap;
    QMap<KJob*, Choqok::Post*> mCreatePostMap;//Job, post
    QMap<KJob*, Choqok::Post*> mFetchPostMap;
    QMap<KJob*, QString> mRequestTimelineMap;//Job, TimelineType
    QMap<QString, QString> mTimelineLatestId;//TimelineType, LatestId

    KUrl mApiUrl;
    uint countOfPost;

// private:
//     TwitterMicroBlog *microBlogInstance;
};

#endif
