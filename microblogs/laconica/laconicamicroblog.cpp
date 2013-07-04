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

#include "laconicamicroblog.h"

#include <KLocale>
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include <KAboutData>
#include <KGenericFactory>
#include "account.h"
#include "accountmanager.h"
#include "microblogwidget.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "laconicaeditaccount.h"
#include "postwidget.h"
#include "laconicaaccount.h"
#include <composerwidget.h>
#include <twitterapihelper/twitterapipostwidget.h>
#include "laconicapostwidget.h"
#include <twitterapihelper/twitterapimicroblogwidget.h>
#include "laconicasearch.h"
#include <kio/netaccess.h>
#include <KMessageBox>
#include <kmimetype.h>
#include "laconicacomposerwidget.h"
#include "mediamanager.h"
#include <choqokappearancesettings.h>
#include "twitterapihelper/twitterapitimelinewidget.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < LaconicaMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_laconica" ) )

LaconicaMicroBlog::LaconicaMicroBlog ( QObject* parent, const QVariantList&  )
: TwitterApiMicroBlog(MyPluginFactory::componentData(), parent), friendsPage(1)
{
    kDebug();
    setServiceName("StatusNet");
    mTimelineInfos["ReTweets"]->name = i18nc("Timeline name", "Repeated");
    mTimelineInfos["ReTweets"]->description = i18nc("Timeline description", "Your posts that were repeated by others");
//     setServiceHomepageUrl("http://twitter.com/");
}

LaconicaMicroBlog::~LaconicaMicroBlog()
{
    kDebug();
}

Choqok::Account * LaconicaMicroBlog::createNewAccount( const QString &alias )
{
    LaconicaAccount *acc = qobject_cast<LaconicaAccount*>( Choqok::AccountManager::self()->findAccount(alias) );
    if(!acc) {
        return new LaconicaAccount(this, alias);
    } else {
        return 0;
    }
}

ChoqokEditAccountWidget * LaconicaMicroBlog::createEditAccountWidget( Choqok::Account *account, QWidget *parent )
{
    kDebug();
    LaconicaAccount *acc = qobject_cast<LaconicaAccount*>(account);
    if(acc || !account)
        return new LaconicaEditAccountWidget(this, acc, parent);
    else{
        kDebug()<<"Account passed here is not a LaconicaAccount!";
        return 0L;
    }
}

Choqok::UI::MicroBlogWidget * LaconicaMicroBlog::createMicroBlogWidget( Choqok::Account *account, QWidget *parent )
{
    return new TwitterApiMicroBlogWidget(account, parent);
}

Choqok::UI::TimelineWidget * LaconicaMicroBlog::createTimelineWidget( Choqok::Account *account,
                                                                 const QString &timelineName, QWidget *parent )
{
    return new TwitterApiTimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget* LaconicaMicroBlog::createPostWidget(Choqok::Account* account,
                                                            Choqok::Post *post, QWidget* parent)
{
    return new LaconicaPostWidget(account, post, parent);
}

Choqok::UI::ComposerWidget* LaconicaMicroBlog::createComposerWidget(Choqok::Account* account, QWidget* parent)
{
    return new LaconicaComposerWidget(account, parent);
}

QString LaconicaMicroBlog::profileUrl( Choqok::Account *account, const QString &username) const
{
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount*>(account);
    if(username.contains('@')){
        QStringList lst = username.split('@', QString::SkipEmptyParts);
        if(lst.count() == 2){
            if(lst[1].endsWith(QString(".status.net"))){
                return QString("http://").arg(lst[1]);
            } else {
                return QString("http://%1/%2").arg(lst[1]).arg(lst[0]);
            }
        }
    }
    if(acc){
        return QString( acc->homepageUrl().prettyUrl(KUrl::AddTrailingSlash) + username) ;
    } else
        return QString();
}

QString LaconicaMicroBlog::postUrl ( Choqok::Account *account,  const QString &username,
                                     const QString &postId ) const
{
    Q_UNUSED(username)
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount*>(account);
    if(acc){
        KUrl url( acc->homepageUrl() );
        url.addPath ( QString("/notice/%1" ).arg ( postId ) );
        return QString ( url.prettyUrl() );
    } else
        return QString();
}

TwitterApiSearch* LaconicaMicroBlog::searchBackend()
{
    if(!mSearchBackend)
        mSearchBackend = new LaconicaSearch(this);
    return mSearchBackend;
}

void LaconicaMicroBlog::createPostWithAttachment(Choqok::Account* theAccount, Choqok::Post* post,
                                                 const QString& mediumToAttach)
{
    if( mediumToAttach.isEmpty() ){
        TwitterApiMicroBlog::createPost(theAccount, post);
    } else {
        QByteArray picData;
        QString tmp;
        KUrl picUrl(mediumToAttach);
        KIO::TransferJob *picJob = KIO::get( picUrl, KIO::Reload, KIO::HideProgressInfo);
        if( !KIO::NetAccess::synchronousRun(picJob, 0, &picData) ){
            kError()<<"Job error: " << picJob->errorString();
            KMessageBox::detailedError(Choqok::UI::Global::mainWindow(),
                                       i18n( "Uploading medium failed: cannot read the medium file." ),
            picJob->errorString() );
            return;
        }
        if ( picData.count() == 0 ) {
            kError() << "Cannot read the media file, please check if it exists.";
            KMessageBox::error( Choqok::UI::Global::mainWindow(),
                                i18n( "Uploading medium failed: cannot read the medium file." ) );
            return;
        }
        ///Documentation: http://identi.ca/notice/17779990
        TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
        KUrl url = account->apiUrl();
        url.addPath ( "/statuses/update.xml" );
        QByteArray fileContentType = KMimeType::findByUrl( picUrl, 0, true )->name().toUtf8();

        QMap<QString, QByteArray> formdata;
        formdata["status"] = post->content.toUtf8();
        formdata["in_reply_to_status_id"] = post->replyToPostId.toLatin1();
        formdata["source"] = "choqok";

        QMap<QString, QByteArray> mediafile;
        mediafile["name"] = "media";
        mediafile["filename"] = picUrl.fileName().toUtf8();
        mediafile["mediumType"] = fileContentType;
        mediafile["medium"] = picData;
        QList< QMap<QString, QByteArray> > listMediafiles;
        listMediafiles.append(mediafile);

        QByteArray data = Choqok::MediaManager::createMultipartFormData(formdata, listMediafiles);

        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        if ( !job ) {
            kError() << "Cannot create a http POST request!";
            return;
        }
        job->addMetaData( "content-type", "Content-Type: multipart/form-data; boundary=AaB03x" );
        job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::POST));
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect( job, SIGNAL( result( KJob* ) ),
                 SLOT( slotCreatePost(KJob*) ) );
        job->start();
    }
}

QString LaconicaMicroBlog::generateRepeatedByUserTooltip(const QString& username)
{
    if( Choqok::AppearanceSettings::showRetweetsInChoqokWay() )
        return i18n("Repeat of %1", username);
    else
        return i18n("Repeated by %1", username);
}

QString LaconicaMicroBlog::repeatQuestion()
{
    return i18n("Repeat this notice?");
}

void LaconicaMicroBlog::listFriendsUsername(TwitterApiAccount* theAccount)
{
    friendsList.clear();
    if ( theAccount ) {
        doRequestFriendsScreenName(theAccount, 1);
    }
}

void LaconicaMicroBlog::requestFriendsScreenName(TwitterApiAccount* theAccount)
{
    doRequestFriendsScreenName(theAccount, 1);
}

void LaconicaMicroBlog::doRequestFriendsScreenName(TwitterApiAccount* theAccount, int page)
{
    kDebug();
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath( QString("/statuses/friends.%1").arg(format));
    QOAuth::ParamMap params;
    if( page > 1 ) {
        params.insert( "page", QByteArray::number( page ) );
        url.addQueryItem( "page", QString::number( page ) );
    }

    KIO::StoredTransferJob *job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url,
                                                                                 QOAuth::GET, params));
    mJobsAccount[job] = theAccount;
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRequestFriendsScreenName(KJob*) ) );
    job->start();
}

void LaconicaMicroBlog::slotRequestFriendsScreenName(KJob* job)
{
    kDebug();
    TwitterApiAccount *theAccount = qobject_cast<TwitterApiAccount *>( mJobsAccount.take(job) );
    KIO::StoredTransferJob* stJob = qobject_cast<KIO::StoredTransferJob*>( job );
    QStringList newList;
    //if(format=="json"){
        newList = readUsersScreenName( theAccount, stJob->data() );
    //} else {
    //    newList = readUsersScreenNameFromXml( theAccount, stJob->data() );
    //}
    friendsList << newList;
    if ( newList.count() == 100 ) {
        doRequestFriendsScreenName( theAccount, ++friendsPage );
    } else {
        friendsList.removeDuplicates();
        theAccount->setFriendsList(friendsList);
        emit friendsUsernameListed( theAccount, friendsList );
    }
}

/*QStringList LaconicaMicroBlog::readUsersScreenNameFromXml(Choqok::Account* theAccount, const QByteArray& buffer)
{
    kDebug();
    QStringList list;
    QDomDocument document;
    document.setContent( buffer );
    QDomElement root = document.documentElement();

    if ( root.tagName() != "users" ) {
        QString err = checkXmlForError(buffer);
        if(!err.isEmpty()){
            emit error(theAccount, ServerError, err, Critical);
        } else {
            err = i18n( "Retrieving the friends list failed. The data returned from the server is corrupted." );
            kDebug() << "there's no users tag in XML\t the XML is: \n" << buffer;
            emit error(theAccount, ParsingError, err, Critical);
            list<<QString(' ');
        }
        return list;
    }
    QDomNode node = root.firstChild();
    QString timeStr;
    while ( !node.isNull() ) {
        if ( node.toElement().tagName() != "user" ) {
            kDebug() << "there's no user tag in XML!\n"<<buffer;
            return list;
        }
        QDomNode node2 = node.firstChild();
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "screen_name" ) {
                list.append( node2.toElement().text() );
                break;
            }
            node2 = node2.nextSibling();
        }
        node = node.nextSibling();
    }
    return list;
}*/

void LaconicaMicroBlog::fetchConversation(Choqok::Account* theAccount, const ChoqokId& conversationId)
{
    kDebug();
    if ( conversationId.isEmpty()) {
        return;
    }
    TwitterApiAccount* account = qobject_cast<TwitterApiAccount*>(theAccount);
    KUrl url = account->apiUrl();
    url.addPath ( QString("/statusnet/conversation/%1.%2").arg(conversationId).arg(format) );

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kDebug() << "Cannot create an http GET request!";
        return;
    }
    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(account, url, QOAuth::GET));
    mFetchConversationMap[ job ] = conversationId;
    mJobsAccount[ job ] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotFetchConversation ( KJob* ) ) );
    job->start();
}

void LaconicaMicroBlog::slotFetchConversation(KJob* job)
{
    kDebug();
    if(!job) {
        kWarning()<<"NULL Job returned";
        return;
    }
    QList<Choqok::Post*> posts;
    ChoqokId conversationId = mFetchConversationMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching conversation failed. %1", job->errorString()), Normal );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> ( job );
        //if(format=="json"){
            posts = readTimeline ( theAccount, stj->data() );
        //} else {
        //    posts = readTimelineFromXml ( theAccount, stj->data() );
        //}
        if( !posts.isEmpty() ){
            emit conversationFetched(theAccount, conversationId, posts);
        }
    }
}

#include "laconicamicroblog.moc"
