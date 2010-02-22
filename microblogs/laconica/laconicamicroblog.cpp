/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < LaconicaMicroBlog > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_laconica" ) )

LaconicaMicroBlog::LaconicaMicroBlog ( QObject* parent, const QVariantList&  )
: TwitterApiMicroBlog(MyPluginFactory::componentData(), parent)
{
    kDebug();
    setServiceName("StatusNet");
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
    return new Choqok::UI::TimelineWidget(account, timelineName, parent);
}

Choqok::UI::PostWidget* LaconicaMicroBlog::createPostWidget(Choqok::Account* account,
                                                        const Choqok::Post &post, QWidget* parent)
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
        KUrl url = apiUrl( qobject_cast<TwitterApiAccount*>(theAccount) );
        url.addPath ( "/statuses/update.xml" );
        QByteArray newLine("\r\n");
        QString formHeader( newLine + "Content-Disposition: form-data; name=\"%1\"" );
        QByteArray header(newLine + "--AaB03x");
        QByteArray footer(newLine + "--AaB03x--");
        QByteArray fileContentType = KMimeType::findByUrl( picUrl, 0, true )->name().toUtf8();
        QByteArray fileHeader(newLine + "Content-Disposition: file; name=\"media\"; filename=\"" +
        picUrl.fileName().toUtf8()+"\"");

        QByteArray data;
//         if ( !post->replyToPostId.isEmpty() && post->content.indexOf ( '@' ) > -1 ) {
//             data += "&in_reply_to_status_id=";
//             data += post->replyToPostId.toLocal8Bit();
//         }
//         data += "&source=choqok";

        data.append(header);
        data.append(fileHeader);
        data.append(newLine + "Content-Type: " + fileContentType);
        data.append(newLine);
        data.append(newLine + picData);

        data.append(header);
        data.append(formHeader.arg("status").toLatin1());
        data.append(newLine);
        data.append(newLine + post->content.toUtf8() );

        data.append(header);
        data.append(formHeader.arg("in_reply_to_status_id").toLatin1());
        data.append(newLine);
        data.append(newLine + post->replyToPostId.toLatin1());

        data.append(header);
        data.append(formHeader.arg("source").toLatin1());
        data.append(newLine);
        data.append(newLine + "choqok");

        data.append(footer);

        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
        if ( !job ) {
            kError() << "Cannot create a http POST request!";
            return;
        }
        job->addMetaData( "content-type", "Content-Type: multipart/form-data; boundary=AaB03x" );
        mCreatePostMap[ job ] = post;
        mJobsAccount[job] = theAccount;
        connect( job, SIGNAL( result( KJob* ) ),
                 SLOT( slotCreatePost(KJob*) ) );
        job->start();
    }
}

#include "laconicamicroblog.moc"
