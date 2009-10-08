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
#ifndef TWITTERAPIMICROBLOGPLUGIN_H
#define TWITTERAPIMICROBLOGPLUGIN_H

#include <microblog.h>
#include <KUrl>
#include "twitterapisearch.h"

class TwitterApiSearchTimelineWidget;
class TwitterApiAccount;
class KJob;

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_HELPER_EXPORT TwitterApiMicroBlog : public Choqok::MicroBlog
{
Q_OBJECT
public:
    ~TwitterApiMicroBlog();

    virtual QMenu* createActionsMenu(Choqok::Account* theAccount,
                                     QWidget* parent = Choqok::UI::Global::mainWindow());
    virtual QList< Choqok::Post* > loadTimeline(Choqok::Account* accountAlias, const QString& timelineName);
    virtual void saveTimeline(Choqok::Account *account, const QString& timelineName,
                              const QList< Choqok::UI::PostWidget* > &timeline);

    /**
    \brief Create a new post

    @see postCreated()
    @see abortCreatePost()
    */
    virtual void createPost( Choqok::Account *theAccount, Choqok::Post *post );

    /**
    \brief Abort all requests!
    */
    virtual void abortAllJobs( Choqok::Account *theAccount );

    /**
    \brief Abort all of createPost requests!
    */
    virtual void abortCreatePost(Choqok::Account* theAccount, Choqok::Post* post = 0);

    /**
    \brief Fetch a post

    @see postFetched()
    */
    virtual void fetchPost( Choqok::Account *theAccount, Choqok::Post *post );

    /**
    \brief Remove a post

    @see postRemoved()
    */
    virtual void removePost( Choqok::Account *theAccount, Choqok::Post *post );

    /**
    Request to update all timelines of account!
    They will arrive in several signals! with timelineDataReceived() signal!

    @see timelineDataReceived()
    */
    virtual void updateTimelines(Choqok::Account *theAccount);

    /**
     add post with Id @p postId to @p theAccount favorite list
    */
    virtual void createFavorite( Choqok::Account *theAccount, const QString &postId );

    /**
     remove post with Id @p postId from @p theAccount favorite list
    */
    virtual void removeFavorite( Choqok::Account *theAccount, const QString &postId );

    virtual void aboutToUnload();

    virtual void listFriendsUsername( TwitterApiAccount *theAccount );

    virtual Choqok::TimelineInfo * timelineInfo(const QString &timelineName);

    /**
    Return search backend to use for search.
    Should be implemented on sub classes
    */
    virtual TwitterApiSearch *searchBackend() = 0;

    virtual TwitterApiSearchTimelineWidget * createSearchTimelineWidget(Choqok::Account* theAccount,
                                                                        QString name, const SearchInfo &info,
                                                                        QWidget *parent);

public slots:
    /**
    Launch a dialog to send direct message.
    There are 2 ways to use this function:
        1. Calling with theAccount option
        2. Get called by a signal from a KAction (Microblog menu)
    */
    virtual void showDirectMessageDialog( TwitterApiAccount *theAccount = 0,
                                          const QString &toUsername = QString() );

    void showSearchDialog( TwitterApiAccount *theAccount = 0 );
signals:
    void favoriteCreated(Choqok::Account *theAccount, const QString &postId);
    void favoriteRemoved(Choqok::Account *theAccount, const QString &postId);
    void friendsUsernameListed( TwitterApiAccount *theAccount, const QStringList &friendsList );

protected slots:
    virtual void slotCreatePost( KJob *job );
    virtual void slotFetchPost( KJob *job );
    virtual void slotRemovePost( KJob *job );
    virtual void slotCreateFavorite( KJob *job );
    virtual void slotRemoveFavorite( KJob *job );
    virtual void slotRequestTimeline( KJob *job );
    virtual void requestFriendsScreenName( TwitterApiAccount* theAccount, int page = 1 );
    virtual void slotRequestFriendsScreenName( KJob *job );

protected:
    TwitterApiMicroBlog( const KComponentData &instance, QObject *parent=0 );
    /**
     Request update for @p timelineName timeline.
     timelineName should be a valid, previously created timeline.
    */
    virtual void requestTimeLine(Choqok::Account *theAccount, QString timelineName,
                                 QString sincePostId, int page = 1, QString maxId = 0 );

    virtual void setTimelineInfos();
    virtual KUrl apiUrl( TwitterApiAccount* theAccount );
    virtual QDateTime dateFromString( const QString &date );
    virtual Choqok::Post * readPostFromDomElement( Choqok::Account* theAccount,
                                                   const QDomElement& root, Choqok::Post* post = 0 );
    virtual Choqok::Post * readPostFromXml( Choqok::Account* theAccount,
                                            const QByteArray& buffer, Choqok::Post* post = 0 );
    virtual QList<Choqok::Post*> readTimelineFromXml( Choqok::Account* theAccount, const QByteArray& buffer );
    virtual Choqok::Post * readDMessageFromXml (Choqok::Account *theAccount, const QByteArray &buffer );
    virtual Choqok::Post * readDMessageFromDomElement (Choqok::Account *theAccount, const QDomElement& root );
    virtual QList<Choqok::Post*> readDMessagesFromXml (Choqok::Account *theAccount, const QByteArray &buffer );
    virtual QStringList readUsersScreenNameFromXml( Choqok::Account *theAccount, const QByteArray & buffer );

    QHash<QString, QString> timelineApiPath;//TimelineType, path
    QMap<QString, Choqok::TimelineInfo*> mTimelineInfos;//timelineName, Info

    QMap<KJob*, QString> mFavoriteMap;//Job, postId
    QMap<KJob*, Choqok::Post*> mRemovePostMap;
    QMap<KJob*, Choqok::Post*> mCreatePostMap;//Job, post
    QMap<KJob*, Choqok::Post*> mFetchPostMap;
    QMap<KJob*, QString> mRequestTimelineMap;//Job, TimelineType
    QHash< Choqok::Account*, QMap<QString, QString> > mTimelineLatestId;//TimelineType, LatestId
    QMap<KJob*, Choqok::Account*> mJobsAccount;

private:
    class Private;
    Private *d;
};

#endif
