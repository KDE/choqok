/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef MICROBLOG_H
#define MICROBLOG_H

#include <QtCore/QString>
#include "plugin.h"
#include "choqok_export.h"
#include "choqoktypes.h"
#include "choqokuiglobal.h"

class ChoqokEditAccountWidget;
class QMenu;

namespace Choqok
{
class Account;
namespace UI{
class PostWidget;
class TimelineWidget;
class MicroBlogWidget;
class ComposerWidget;
}
/**
\brief Base class for MicroBlog plugins
Every MicroBlog plugin should subclass this they can subclass UI classes to use too, and implement:
@ref createNewAccount() To create a new account for this microblog, can use @ref Choqok::Account or a subclass of it,
@ref createEditAccountWidget() To create a widget/dialog to show to user for create/edit account,
@ref createMicroBlogWidget() To create a @ref MicroBlogWidget to show on MainWindow
@ref createTimelineWidget() To create a @ref TimelineWidget to show on MicroBlogWidget

Additionally should implement this functions:
@ref createPost() @ref abortCreatePost() @ref fetchPost() @ref removePost() @ref updateTimelines() @ref profileUrl()

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT MicroBlog : public Plugin
{
Q_OBJECT
public:
    virtual ~MicroBlog();

    /**
    @brief Return a KMenu contain microblog specific actions.
    Can use for search facilities, or other things
    Will add to a button on top of @ref MicroBlogWidget of account/microblog
    */
    virtual QMenu *createActionsMenu( Account *theAccount, QWidget *parent = UI::Global::mainWindow() );

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
    Enumeration for levels of errors.
    This could use for user feedback!
    For @ref Critical errors will show a messagebox,
    For @ref Normal will show a knotify.
    For @ref Low just show a message in statusBar
    */
    enum ErrorLevel {
        Low = 0,
        Normal,
        Critical
    };

    /**
    * @brief Create a new Account
    *
    * This method is called during the loading of the config file.
    * @param alias - the alias name to create the account with.
    *
    * you don't need to register the account to the AccountManager in this function.
    *
    * @return The new @ref Account object created by this function
    */
    virtual Account *createNewAccount( const QString &alias );

    /**
    * @brief Create a new EditAccountWidget
    *
    * @return A new @ref ChoqokEditAccountWidget to be shown in the account part of the configurations.
    *
    * @param account is the Account to edit. If it's 0L, then we create a new account
    * @param parent The parent of the 'to be returned' widget
    */
    virtual ChoqokEditAccountWidget * createEditAccountWidget( Choqok::Account *account, QWidget *parent ) = 0;

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
    virtual UI::MicroBlogWidget * createMicroBlogWidget( Choqok::Account *account, QWidget *parent );

    /**
    * @brief Create a ComposerWidget to use in MicroBlogWidget
    *
    * @return A new ComposerWidget to use.
    *
    * @param account account to use.
    * @param parent The parent of the 'to be returned' widget
    */
    virtual UI::ComposerWidget * createComposerWidget( Choqok::Account *account, QWidget *parent );

    /**
    * @brief Create a TimelineWidget to use in MicroBlogWidget
    *
    * @return A new TimelineWidget to use.
    *
    * @param account account to use.
    * @param timelineName Name of timeline
    * @param parent The parent of the 'to be returned' widget
    */
    virtual UI::TimelineWidget * createTimelineWidget( Choqok::Account *account, const QString &timelineName,
                                                   QWidget *parent );

    /**
     * @brief Create a PostWidget to contain one post in TimelineWidget
     *
     * @return A new PostWidget to use.
     *
     * @param account account to use.
     * @param post Post object.
     * @param parent The parent of the 'to be returned' widget
     */
    virtual UI::PostWidget * createPostWidget( Choqok::Account *account,
                                               Choqok::Post *post, QWidget *parent );

    /**
    @brief Save a specific timeline!
    @Note Implementation of this is optional, i.e. One microblog may don't have timeline backup

    @see loadTimeline()
    */
    virtual void saveTimeline( Choqok::Account *account, const QString &timelineName,
                               const QList<UI::PostWidget*> &timeline);
    /**
    @brief Load a specific timeline!
    @Note Implementation of this is optional, i.e. One microblog may don't have timeline backup

    @see saveTimeline()
    */
    virtual QList<Post*> loadTimeline( Choqok::Account *account, const QString &timelineName );

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
    \brief Abort all createPost jobs
    \see abortAllJobs()
    */
    virtual void abortCreatePost( Choqok::Account *theAccount, Choqok::Post *post = 0 );
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
    virtual void updateTimelines( Choqok::Account *theAccount );

    /**
    return Url to account page on service (Some kind of blog homepage)
    */
    virtual QString profileUrl( Choqok::Account *account, const QString &username) const;

    /**
    Provide a Web link for post with id @p postId
    */
    virtual QString postUrl( Choqok::Account *account, const QString &username, const QString &postId) const;

    /**
    Return a list of timelines supported by this account!
    It will use to show timelines! and result of timelineDataReceived() signal will be based on these!

    @see timelineInfo()
    @see timelineDataReceived()
    */
    QStringList timelineNames() const;

    /**
    Checks if @p timeline is valid for this blog! i.e. there is an entry for it at timelineTypes() list.
    @return True if the timeline is valid, false if not!
    */
    bool isValidTimeline( const QString &timelineName );

    /**
    @Return information about an specific timeline
    */
    virtual TimelineInfo * timelineInfo( const QString &timelineName );

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

Q_SIGNALS:

    /**
    Emit when data for a timeline received! @p type specifies the type of timeline as specifies in timelineTypes()
    */
    void timelineDataReceived( Choqok::Account *theAccount, const QString &timelineName,
                               QList<Choqok::Post*> data );

    /**
    Emit when a post successfully created!
    */
    void postCreated( Choqok::Account *theAccount, Choqok::Post *post );

    /**
    Emit when a post successfully fetched!
    */
    void postFetched( Choqok::Account *theAccount, Choqok::Post *post );

    /**
    Emit when a post successfully removed!
    */
    void postRemoved( Choqok::Account *theAccount, Choqok::Post *post );

    /**
    Emit when an error occurred the @p errorMessage will specify the error.
    */
    void error( Choqok::Account *theAccount, Choqok::MicroBlog::ErrorType error,
                const QString &errorMessage, Choqok::MicroBlog::ErrorLevel level = Normal );

    /**
    Emit when an error occurred on Post manipulation. e.g. On Creation!
    */
    void errorPost( Choqok::Account *theAccount, Choqok::Post *post,
                    Choqok::MicroBlog::ErrorType error, const QString &errorMessage,
                    Choqok::MicroBlog::ErrorLevel level = Normal );

    /**
    Emit when microblog plugin is going to unload, and @ref Choqok::TimelineWidget should save their timelines
    */
    void saveTimelines();

protected:

    MicroBlog( const KComponentData &instance, QObject *parent=0 );

    virtual void setTimelineNames(const QStringList&);
    void addTimelineName(const QString&);
    void setServiceName(const QString&);
    void setServiceHomepageUrl(const QString&);
    void setCharLimit(uint);

protected slots:
    void slotConfigChanged();

private:
    class Private;
    Private * const d;
};

}

#endif
