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

#ifndef MICROBLOGPLUGIN_H
#define MICROBLOGPLUGIN_H

#include "plugin.h"
#include <QtCore/QString>
#include <QString>
#include "choqok_export.h"
#include "choqoktypes.h"

class ChoqokEditAccountWidget;

namespace Choqok
{
class PostWidget;
class TimelineWidget;
class MicroBlogWidget;
class Account;

/**
\brief Base class for MicroBlog plugins
Every MicroBlog plugin should subclass this they can subclass UI classes to use too, and implement:
@ref createAccount() To create an account for this microblog, can use @ref Choqok::Account or a subclass of it,
@ref createEditAccountWidget() To create a widget/dialog to show to user for create/edit account,
@ref createMicroBlogWidget() To create a @ref MicroBlogWidget to show on MainWindow
@ref createTimelineWidget() To create a @ref TimelineWidget to show on MicroBlogWidget

Additionally should implement this functions:
@ref createPost() @ref abortCreatePost() @ref fetchPost() @ref removePost() @ref updateTimelines() @ref profileUrl()
*/
class CHOQOK_EXPORT MicroBlog : public Plugin
{
Q_OBJECT
public:
    virtual ~MicroBlog();

    /**
    Enumeration for possible errors.
    */
    enum ErrorType {
        /** A server side error. */
        ServerError,
        /** An error on communication with server */
        CommunicationError,
        /** A parsing error. */
        ParsingError,
        /** An error on authentication. */
        AuthenticationError,
        /** An error where the method called is not supported by this object. */
        NotSupportedError,
        /** Any other miscellaneous error. */
        OtherError
    };

    /**
    * @brief Create an empty Account
    *
    * This method is called during the loading of the config file.
    * @param accountId - the account ID to create the account with. This is usually
    * the login name of the account
    *
    * you don't need to register the account to the AccountManager in this function.
    *
    * @return The new @ref Account object created by this function
    */
    virtual Account *createAccount( const QString &alias ) = 0;

    /**
    * @brief Create a new EditAccountWidget
    *
    * @return A new Choqok::EditAccountWidget to be shown in the account part of the configurations.
    *
    * @param account is the Account to edit. If it's 0L, then we create a new account
    * @param parent The parent of the 'to be returned' widget
    */
    virtual ChoqokEditAccountWidget * createEditAccountWidget( Account *account, QWidget *parent ) = 0;

    /**
    * @brief Create a MicroBlogWidget for this Account
    * The returned MicroBlogWidget will show on Mainwindow. and manage of this microblog account will give to it
    * Every MicroBlog plugin should reimplement this.
    *
    * @return A new MicroBlogWidget to use.
    *
    * @param account account to use.
    * @param parent The parent of the 'to be returned' widget
    */
    virtual MicroBlogWidget * createMicroBlogWidget( Account *account, QWidget *parent ) = 0;

    /**
    * @brief Create a TimelineWidget to use in MicroBlogWidget
    *
    * @return A new TimelineWidget to use.
    *
    * @param account account to use.
    * @param parent The parent of the 'to be returned' widget
    */
    virtual TimelineWidget * createTimelineWidget( Account *account, const QString &timelineName,
                                                   QWidget *parent ) = 0;

    /**
     * @brief Create a PostWidget to contain one post in TimelineWidget
     *
     * @return A new PostWidget to use.
     *
     * @param account account to use.
     * @param parent The parent of the 'to be returned' widget
     */
    virtual PostWidget * createPostWidget( Account *account, const Post &post, QWidget *parent ) = 0;

    /**
    @brief Save a specific timeline!
    @Note Implementation of this is optional, i.e. One microblog may don't have timeline backup

    @see loadTimeline()
    */
    virtual void saveTimeline( const QString &accountAlias, const QString &timelineName,
                               QList<PostWidget*> timeline) = 0;
    /**
    @brief Load a specific timeline!
    @Note Implementation of this is optional, i.e. One microblog may don't have timeline backup

    @see saveTimeline()
    */
    virtual QList<Post*> loadTimeline( const QString &accountAlias, const QString &timelineName ) = 0;

    void setCurrentAccount(Choqok::Account* account);
    Account * currentAccount() const;
public slots:

    /**
    \brief Create a new post

    @see postCreated()
    @see abortCreatePost()
    */
    virtual void createPost( Post *post ) = 0;

    /**
    \brief Abort all of createPost requests!
    */
    virtual void abortCreatePost() = 0;

    /**
    \brief Fetch a post

    @see postFetched()
    */
    virtual void fetchPost( Post *post ) = 0;

    /**
    \brief Remove a post

    @see postRemoved()
    */
    virtual void removePost( Post *post ) = 0;

    /**
    Request to update all timelines of account!
    They will arrive in several signals! with timelineDataReceived() signal!

    @see timelineDataReceived()
    */
    virtual void updateTimelines() = 0;

signals:

    /**
    Emit when data for a timeline recieved! @p type specifies the type of timeline as specifies in timelineTypes()
    */
    void timelineDataReceived( const QString &type, QList<Choqok::Post*> data );

    /**
    Emit when a post successfully created!
    */
    void postCreated( Post *post );

    /**
    Emit when a post successfully fetched!
    */
    void postFetched( Post *post );

    /**
    Emit when a post successfully removed!
    */
    void postRemoved( Post *post );

    /**
    Emit when an error occured the @p errorMessage will specify the error.
    */
    void error( Choqok::MicroBlog::ErrorType error, const QString &errorMessage );

    /**
    Emit when an error occured on Post manipulation. e.g. On Creation!
    */
    void errorPost( Choqok::MicroBlog::ErrorType error, const QString &errorMessage,
                    const Post *post );

public:
    /**
    return Url to account page on service (Some kind of blog homepage)
    */
    virtual QString profileUrl(const QString &username) const=0;

    virtual QString postUrl(const QString &username, const QString &postId) = 0;

    /**
    * @brief Available capabilities
    *
    * @ref capabilities() returns an ORed list of these, which
    * the edit widget interperts to determine what buttons to show
    */
    enum Capability
    {
        CreatPost = 0x1,
        RemovePost = 0x3
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    /**
    * @brief a bitmask of the capabilities of this protocol
    * @sa @ref setCapabilities
    */
    Capabilities capabilities() const;

    /**
    Return a list of timelines supported by this account!
    It will use to show timelines! and result of timelineDataReceived() signal will be based on these!

    @see timelineInfo()
    @see timelineDataReceived()
    */
    QStringList timelineTypes() const;

    /**
    Checks if @p timeline is valid for this blog! i.e. there is an entry for it at timelineTypes() list.
    @return True if the timeline is valid, false if not!
    */
    bool isValidTimeline( const QString &timeline );

    /**
    Return service homepage Url
    */
    QString homepageUrl() const;

    /**
    Returns a user readable name of blog/service type! (e.g. Identica)
    */
    QString serviceName() const;

    static QString errorString( ErrorType type );

    /**
    Indicate character limit for a post. 0 means no limit.
    */
    uint postCharLimit() const;

protected:

    MicroBlog( const KComponentData &instance, QObject *parent=0 );

    /**
    * @brief Sets the capabilities of this microblog.
    *
    * The subclass contructor is a good place for calling it.
    * @sa @ref capabilities()
    */
    void setCapabilities( Capabilities );
    void setTimelineTypes(const QStringList&);
    void setServiceName(const QString&);
    void setServiceHomepageUrl(const QString&);
//     void setError(int);
//     void setErrorString(const QString&);
    void setCharLimit(uint);

private:
    class Private;
    Private * const d;
};

}

#endif
