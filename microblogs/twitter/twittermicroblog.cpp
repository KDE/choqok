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

#include "twittermicroblog.h"

#include <KLocale>
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <QDomElement>
#include <KAboutData>
#include <KGenericFactory>
#include "account.h"
#include "accountmanager.h"
#include "timelinewidget.h"
#include "editaccountwidget.h"
#include "twittereditaccount.h"
#include "postwidget.h"
#include "twitteraccount.h"
#include "composerwidget.h"
#include "twitterpostwidget.h"
#include "twitterapihelper/twitterapimicroblogwidget.h"
#include "twittersearch.h"
#include "twitterapihelper/twitterapicomposerwidget.h"
#include "choqokappearancesettings.h"
#include <QMenu>
#include <KAction>
#include "twitterlistdialog.h"
#include <KMessageBox>
#include <qjson/parser.h>
#include "choqoktypes.h"
#include "twittertimelinewidget.h"
#include "twittercomposerwidget.h"
#include <kio/netaccess.h>
#include <kmimetype.h>
#include "mediamanager.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < TwitterMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_twitter" ) )

TwitterMicroBlog::TwitterMicroBlog ( QObject* parent, const QVariantList&  )
: TwitterApiMicroBlog(MyPluginFactory::componentData(), parent)
{
    kDebug();
    setServiceName("Twitter");
    setServiceHomepageUrl("https://twitter.com/");
    timelineApiPath["Reply"] = "/statuses/mentions_timeline.%1";
    setTimelineInfos();
}
void TwitterMicroBlog::setTimelineInfos()
{
 //   hange description of replies to mentions
    Choqok::TimelineInfo *t = mTimelineInfos["Reply"];    
    t->name = i18nc("Timeline Name", "Mentions");
    t->description = i18nc("Timeline description", "Mentions of you");
}

TwitterMicroBlog::~TwitterMicroBlog()
{
    kDebug();
}

Choqok::Account * TwitterMicroBlog::createNewAccount( const QString &alias )
{
    TwitterAccount *acc = qobject_cast<TwitterAccount*>( Choqok::AccountManager::self()->findAccount(alias) );
    if(!acc) {
        return new TwitterAccount(this, alias);
    } else {
        return 0;
    }
}

ChoqokEditAccountWidget * TwitterMicroBlog::createEditAccountWidget( Choqok::Account *account, QWidget *parent )
{
    kDebug();
    TwitterAccount *acc = qobject_cast<TwitterAccount*>(account);
    if(acc || !account)
        return new TwitterEditAccountWidget(this, acc, parent);
    else{
        kDebug()<<"Account passed here is not a TwitterAccount!";
        return 0L;
    }
}

Choqok::UI::MicroBlogWidget * TwitterMicroBlog::createMicroBlogWidget( Choqok::Account *account, QWidget *parent )
{
    return new TwitterApiMicroBlogWidget(account, parent);
}

Choqok::UI::TimelineWidget * TwitterMicroBlog::createTimelineWidget( Choqok::Account *account,
                                                                 const QString &timelineName, QWidget *parent )
{
    return new TwitterTimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget* TwitterMicroBlog::createPostWidget(Choqok::Account* account,
                                                           Choqok::Post *post, QWidget* parent)
{
    return new TwitterPostWidget(account, post, parent);
}

Choqok::UI::ComposerWidget* TwitterMicroBlog::createComposerWidget(Choqok::Account* account, QWidget* parent)
{
    return new TwitterComposerWidget(account, parent);
}

QString TwitterMicroBlog::profileUrl(Choqok::Account*, const QString& username) const
{
    return QString( "https://twitter.com/#!/%1" ).arg( username );
}

QString TwitterMicroBlog::postUrl(Choqok::Account*, const QString& username,
                                  const QString& postId) const
{
    return QString ( "https://twitter.com/%1/status/%2" ).arg ( username ).arg ( postId );
}

TwitterApiSearch* TwitterMicroBlog::searchBackend()
{
    if(!mSearchBackend)
        mSearchBackend = new TwitterSearch(this);
    return mSearchBackend;
}

void TwitterMicroBlog::createPostWithAttachment(Choqok::Account* theAccount, Choqok::Post* post,
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
        TwitterAccount* account = qobject_cast<TwitterAccount*>(theAccount);
        KUrl url = account->uploadUrl();
        url.addPath ( "/statuses/update_with_media.json" );
        QByteArray fileContentType = KMimeType::findByUrl( picUrl, 0, true )->name().toUtf8();

        QMap<QString, QByteArray> formdata;
        formdata["status"] = post->content.toUtf8();
        if (!post->replyToPostId.isEmpty()) {
            formdata["in_reply_to_status_id"] = post->replyToPostId.toLatin1();
        }
        formdata["source"] = "choqok";

        QMap<QString, QByteArray> mediafile;
        mediafile["name"] = "media[]";
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

QString TwitterMicroBlog::generateRepeatedByUserTooltip(const QString& username)
{
    if( Choqok::AppearanceSettings::showRetweetsInChoqokWay() )
        return i18n("Retweet of %1", username);
    else
        return i18n("Retweeted by %1", username);
}

QString TwitterMicroBlog::repeatQuestion()
{
    return i18n("Retweet to your followers?");
}

QMenu* TwitterMicroBlog::createActionsMenu(Choqok::Account* theAccount, QWidget* parent)
{
    QMenu *menu = TwitterApiMicroBlog::createActionsMenu(theAccount, parent);

    KAction *lists = new KAction( i18n("Add User List..."), menu );
    lists->setData( theAccount->alias() );
    connect( lists, SIGNAL(triggered(bool)), SLOT(showListDialog()) );
    menu->addAction(lists);

    return menu;
}

void TwitterMicroBlog::showListDialog(TwitterApiAccount* theAccount)
{
    if( !theAccount ) {
        KAction *act = qobject_cast<KAction *>(sender());
        theAccount = qobject_cast<TwitterApiAccount*>(
                                    Choqok::AccountManager::self()->findAccount( act->data().toString() ) );
    }
    QPointer<TwitterListDialog> listDlg = new TwitterListDialog( theAccount,
                                                                 Choqok::UI::Global::mainWindow() );
    listDlg->show();
}

void TwitterMicroBlog::fetchUserLists(TwitterAccount* theAccount, const QString& username)
{
    kDebug();
    if ( !theAccount) {
        return;
    }
    KUrl url = theAccount->apiUrl();
    url.addPath ( "/lists/ownerships.json" );
    KUrl url_for_oauth(url);//we need base URL (without params) to make OAuth signature with it!
    url.addQueryItem("screen_name", username);
    QOAuth::ParamMap params;
    params.insert("screen_name", username.toLatin1());

    KIO::StoredTransferJob *job = KIO::storedGet ( url, KIO::Reload, KIO::HideProgressInfo ) ;
    if ( !job ) {
        kError() << "TwitterMicroBlog::loadUserLists: Cannot create an http GET request!";
        return;
    }

    job->addMetaData("customHTTPHeader", "Authorization: " + authorizationHeader(theAccount, url_for_oauth, QOAuth::GET, params));
    mFetchUsersListMap[ job ] = username;
    mJobsAccount[ job ] = theAccount;
    connect ( job, SIGNAL ( result ( KJob* ) ), this, SLOT ( slotFetchUserLists(KJob*) ) );
    job->start();
}

void TwitterMicroBlog::slotFetchUserLists(KJob* job)
{
    kDebug();
    if(!job) {
        kWarning()<<"NULL Job returned";
        return;
    }
    QString username = mFetchUsersListMap.take(job);
    Choqok::Account *theAccount = mJobsAccount.take(job);
    if ( job->error() ) {
        kDebug() << "Job Error: " << job->errorString();
        emit error ( theAccount, Choqok::MicroBlog::CommunicationError,
                     i18n("Fetching %1's lists failed. %2", username, job->errorString()), Critical );
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> ( job );
        QByteArray buffer = stj->data();
        QList<Twitter::List> list = readUserListsFromJson ( theAccount, buffer );
        if ( list.isEmpty() ) {
            kDebug() << buffer;
            QString errorMsg;
            errorMsg = checkForError(buffer);
            if( errorMsg.isEmpty() ){
                KMessageBox::information(choqokMainWindow, i18n("There is no list record for user %1", username));
            } else {
                emit error( theAccount, ServerError, errorMsg, Critical);
            }
        } else {
            emit userLists(theAccount, username, list);
        }
    }
}

void TwitterMicroBlog::addListTimeline( TwitterAccount* theAccount, const QString& username,
                                        const QString& listname )
{
    kDebug();
    QStringList tms = theAccount->timelineNames();
    QString name = QString("@%1/%2").arg(username).arg(listname);
    tms.append(name);
    addTimelineName(name);
    theAccount->setTimelineNames(tms);
    theAccount->writeConfig();
    QString url = QString("/%1/lists/%2/statuses").arg(username).arg(listname);
    timelineApiPath[name] = url + ".%1";
    updateTimelines(theAccount);
}

void TwitterMicroBlog::setListTimelines(TwitterAccount* theAccount, const QStringList& lists)
{
    kDebug()<<lists;
    QStringList tms = theAccount->timelineNames();
    foreach(const QString &name, lists){
        QString lst = name;
        lst.remove(0, 1);
        QStringList userlist = lst.split('/');

        QString username = userlist[0], listname = userlist[1];
        tms.append(name);
        addTimelineName(name);
        QString url = QString("/%1/lists/%2/statuses").arg(username).arg(listname);
        timelineApiPath[name] = url + ".%1";
    }
    tms.removeDuplicates();
    theAccount->setTimelineNames(tms);
}

Choqok::TimelineInfo* TwitterMicroBlog::timelineInfo(const QString& timelineName)
{
    if(timelineName.startsWith('@')){
        if(mListsInfo.contains(timelineName)) {
            return mListsInfo.value(timelineName);
        } else {
            Choqok::TimelineInfo *info = new Choqok::TimelineInfo;
            info->description = info->name = timelineName;
            info->icon = "format-list-unordered";
            mListsInfo.insert(timelineName, info);
            return info;
        }
    } else {
        return TwitterApiMicroBlog::timelineInfo(timelineName);
    }
}

QList< Twitter::List > TwitterMicroBlog::readUserListsFromJson(Choqok::Account* theAccount, QByteArray buffer)
{
    bool ok;
    QList<Twitter::List> twitterList;
    QVariantMap map = parser()->parse(buffer, &ok).toMap();

    if ( ok && map.contains("lists") ) {
        QVariantList list = map["lists"].toList();
        QVariantList::const_iterator it = list.constBegin();
        QVariantList::const_iterator endIt = list.constEnd();
        for(; it != endIt; ++it){
            twitterList.append(readListFromJsonMap(theAccount, it->toMap()));
        }
    }
    return twitterList;
}

Twitter::List TwitterMicroBlog::readListFromJsonMap(Choqok::Account* theAccount, QVariantMap map)
{
    Twitter::List l;
    l.author = readUser( theAccount, map["user"].toMap() );
    l.description = map["description"].toString();
    l.fullname = map["full_name"].toString();
    l.isFollowing = map["following"].toBool();
    l.listId = map["id"].toString();
    l.memberCount = map["member_count"].toInt();
    l.mode = (map["mode"].toString() == "public" ? Twitter::Public : Twitter::Private);
    l.name = map["name"].toString();
    l.slug = map["slug"].toString();
    l.subscriberCount = map["subscriber_count"].toInt();
    l.uri = map["uri"].toString();
    return l;
}

#include "twittermicroblog.moc"
