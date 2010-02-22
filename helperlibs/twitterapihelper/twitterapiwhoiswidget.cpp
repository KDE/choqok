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

#include "twitterapiwhoiswidget.h"
#include "mediamanager.h"
#include <KTextBrowser>
#include <KUrl>
#include <kicon.h>
#include <KNotification>
#include <KProcess>
#include <KToolInvocation>
#include <KApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include "twitterapiaccount.h"
#include <KIO/Job>
#include <KDebug>
#include <QDomDocument>
#include <klocalizedstring.h>
#include <kanimatedbutton.h>
// <a href='choqok://follow'>%6</a>
#include <choqoktypes.h>
#include <microblog.h>
#include "twitterapimicroblog.h"
#include <choqokappearancesettings.h>
#include <notifymanager.h>
#include <choqoktools.h>
#include <kstatusbar.h>

const char * baseText = "\
<table width=\"100%\">\
    <tr>\
        <td>\
            <b>Who is <i>%1</i> ?</b> %10\
            <a href='choqok://close'><img src='icon://close' title='Close' align='right' /></a>\
        </td>\
    </tr>\
    <tr>\
        <td>\
            <table>\
                <tr>\
                    <td>\
                        <table><tr>\
                            <td width=\"48\">\
                                <img width=48 height=48 src='img://profileImage'/>\
                            </td>\
                            <td>\
                                <b>Name:</b> %2<br/>\
                                <b>Location:</b> %3<br/>\
                                <b>TimeZone:</b> %4\
                            </td>\
                        </tr></table>\
                    </td>\
                </tr>\
                <tr>\
                    <td>\
                        <b>Web:</b> %5<br/>\
                        <b>Bio:</b> %6<br/>\
                        <b>Last Status: </b>%7<br/><br/>\
                        %8 Friends!<br/>\
                        %9 Followers!\
                        \
                    </td>\
                </tr>\
            </table>\
        </td>\
    </tr>\
</table>";

class TwitterApiWhoisWidget::Private
{
public:
    Private(TwitterApiAccount *account, const QString &userN)
    :currentAccount(account), waitFrame(0), job(0), username(userN)
    {
        mBlog = qobject_cast<TwitterApiMicroBlog*>(account->microblog());
    }
    KTextBrowser *wid;
    TwitterApiAccount *currentAccount;
    TwitterApiMicroBlog *mBlog;
    QFrame *waitFrame;
    KJob *job;
    Choqok::Post currentPost;
    QString username;

    QString followersCount;
    QString friendsCount;
    QString timeZone;
    QString imgActions;
//     bool isFollowing;
};

TwitterApiWhoisWidget::TwitterApiWhoisWidget(TwitterApiAccount* theAccount, const QString& username,
                                             QWidget* parent)
    : QFrame(parent), d(new Private(theAccount, username))
{
    kDebug();
    setAttribute(Qt::WA_DeleteOnClose);

    loadUserInfo(theAccount, username);

    d->wid = new KTextBrowser(this);
    setFrameShape(StyledPanel);
    setFrameShadow(Sunken);

    d->wid->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(d->wid);
    this->setLayout(layout);
    this->setWindowFlags(Qt::Popup);// | Qt::FramelessWindowHint | Qt::Ta);
    d->wid->setOpenLinks(false);
    connect(d->wid,SIGNAL(anchorClicked(const QUrl)),this,SLOT(checkAnchor(const QUrl)));
    setupUi();
    setActionImages();
}

TwitterApiWhoisWidget::~TwitterApiWhoisWidget()
{
    kDebug();
    delete d;
}

void TwitterApiWhoisWidget::loadUserInfo(TwitterApiAccount* theAccount, const QString& username)
{
    kDebug();
    KUrl url( theAccount->apiUrl() );
    url.setScheme ( theAccount->useSecureConnection() ? "https" : "http" );
    url.setUser ( theAccount->username() );
    url.setPass ( theAccount->password() );

    url.addPath( QString( "/users/show/%1.xml" ).arg(username));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    d->job = job;
    connect( job, SIGNAL(result(KJob*)), SLOT(userInfoReceived(KJob*)));
    job->start();
}

void TwitterApiWhoisWidget::userInfoReceived(KJob* job)
{
    kDebug();
    if(job->error()){
        kError()<<"Job Error: "<<job->errorString();
        if( Choqok::UI::Global::mainWindow()->statusBar() )
            Choqok::UI::Global::mainWindow()->statusBar()->showMessage(job->errorString());
        slotCancel();
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    QDomDocument doc;
    doc.setContent(stj->data());

    QDomElement root = doc.documentElement();
    if ( root.tagName() != "user" ) {
        kDebug()<<"There's no user tag in returned document from server! Data is:\n\t"<<stj->data();
        d->wid->setText(i18n("Cannot load user information."));
        return;
    }
    QDomNode node = root.firstChild();
    Choqok::Post post;
    QString timeStr;
    while( !node.isNull() ){
        QDomElement elm = node.toElement();
        if(elm.tagName() == "name"){
            post.author.realName = elm.text();
        } else if(elm.tagName() == "screen_name"){
            post.author.userName = elm.text();
        } else if(elm.tagName() == "location"){
            post.author.location = elm.text();
        } else if(elm.tagName() == "description"){
            post.author.description = elm.text();
        } else if(elm.tagName() == "profile_image_url"){
            post.author.profileImageUrl = elm.text();
        } else if(elm.tagName() == "url") {
            post.author.homePageUrl = elm.text();
        } else if(elm.tagName() == "time_zone") {
            d->timeZone = elm.text();
        } else if(elm.tagName() == "followers_count") {
            d->followersCount = elm.text();
        } else if(elm.tagName() == "friends_count") {
            d->friendsCount = elm.text();
        }/* else if( elm.tagName() == "protected" ){
            if(elm.text() == "true"){
                d->lockImg = "<img src=\"icon://lock\">";
                d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("icon://lock"),
                                                 KIcon("object-locked").pixmap(16) );
            }
        }*/ else if(elm.tagName() == "status") {
            QDomNode node2 = elm.firstChild();
            while( !node2.isNull() ){
                QDomElement elm2 = node2.toElement();
                if ( elm2.tagName() == "created_at" )
                    timeStr = elm2.text();
                else if ( elm2.tagName() == "text" )
                    post.content = elm2.text();
                else if ( elm2.tagName() == "id" )
                    post.postId = elm2.text();
                else if ( elm2.tagName() == "in_reply_to_status_id" )
                    post.replyToPostId = elm2.text();
                else if ( elm2.tagName() == "in_reply_to_user_id" )
                    post.replyToUserId = elm2.text();
                else if ( elm2.tagName() == "in_reply_to_screen_name" )
                    post.replyToUserName = elm2.text();
                else if ( elm2.tagName() == "source" )
                    post.source = elm2.text();
                else if ( elm2.tagName() == "favorited" )
                    post.isFavorited = ( elm2.text() == "true" ) ? true : false;
                node2 = node2.nextSibling();
            }
        }
        node = node.nextSibling();
    }
//     post.link = d->currentAccount->microblog()->postUrl(d->currentAccount, post.author.userName, post.postId);
//     TwitterApiMicroBlog *blog = qobject_cast<TwitterApiMicroBlog *>(d->currentAccount->microblog());
//     post.creationDateTime = blog->dateFromString( timeStr );

    d->currentPost = post;
    updateHtml();
    showForm();

    QPixmap *userAvatar = Choqok::MediaManager::self()->fetchImage( post.author.profileImageUrl,
                                                                    Choqok::MediaManager::Async );

    if(userAvatar) {
        d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"),
                                         *(userAvatar) );
    } else {
        connect( Choqok::MediaManager::self(), SIGNAL( imageFetched(QString,QPixmap)),
                this, SLOT(avatarFetched(QString, QPixmap) ) );
        connect( Choqok::MediaManager::self(), SIGNAL(fetchError(QString,QString)),
                this, SLOT(avatarFetchError(QString,QString)) );
    }
}

void TwitterApiWhoisWidget::avatarFetched(const QString& remoteUrl, const QPixmap& pixmap)
{
    kDebug();
    if ( remoteUrl == d->currentPost.author.profileImageUrl ) {
        QString url = "img://profileImage";
        d->wid->document()->addResource( QTextDocument::ImageResource, url, pixmap );
        updateHtml();
        disconnect( Choqok::MediaManager::self(), SIGNAL( imageFetched(QString,QPixmap)),
                    this, SLOT(avatarFetched(QString, QPixmap) ) );
        disconnect( Choqok::MediaManager::self(), SIGNAL(fetchError(QString,QString)),
                    this, SLOT(avatarFetchError(QString,QString))  );
    }
}

void TwitterApiWhoisWidget::avatarFetchError(const QString& remoteUrl, const QString& errMsg)
{
    kDebug();
    Q_UNUSED(errMsg);
    if( remoteUrl == d->currentPost.author.profileImageUrl ){
        ///Avatar fetching is failed! but will not disconnect to get the img if it fetches later!
        QString url = "img://profileImage";
        d->wid->document()->addResource( QTextDocument::ImageResource, url, KIcon("image-missing").pixmap(48) );
        updateHtml();
    }
}

void TwitterApiWhoisWidget::updateHtml()
{
    kDebug();
    QString url = d->currentPost.author.homePageUrl.isEmpty() ? QString()
                : QString("<a title='%1' href='%1'>%1</a>").arg(d->currentPost.author.homePageUrl);

//     QString dir = post.content.isRightToLeft() ? "rtl" : "ltr";
    QString tmpStr = i18n(baseText);
    QString html = QString(tmpStr).arg(d->currentPost.author.userName)
                                    .arg(d->currentPost.author.realName)
                                    .arg(d->currentPost.author.location)
                                    .arg(d->timeZone)
                                    .arg(url)
                                    .arg(d->currentPost.author.description)
                                    .arg(d->currentPost.content)//.arg(dir);
                                    .arg(d->friendsCount)
                                    .arg(d->followersCount)
                                    .arg(d->imgActions);

    d->wid->setHtml(html);
}

void TwitterApiWhoisWidget::showForm()
{
    kDebug();
    QPoint pos = d->waitFrame->pos();
    d->waitFrame->deleteLater();
    d->wid->resize(320, 200);
    d->wid->document()->setTextWidth(width()-2);
    int h = d->wid->document()->size().toSize().height() + 10;
    d->wid->setMinimumHeight(h);
    d->wid->setMaximumHeight(h);
    this->resize(320,h+4);
    int desktopHeight = KApplication::desktop()->height();
    int desktopWidth = KApplication::desktop()->width();
    if( (pos.x() + this->width()) > desktopWidth )
        pos.setX(desktopWidth - width());
    if( (pos.y() + this->height()) > desktopHeight )
        pos.setY(desktopHeight - height());
    move(pos);
    QWidget::show();
}

void TwitterApiWhoisWidget::show(QPoint pos)
{
    kDebug();
    d->waitFrame = new QFrame(this);
    d->waitFrame->setFrameShape(NoFrame);
    d->waitFrame->setWindowFlags(Qt::Popup);
    KAnimatedButton *waitButton = new KAnimatedButton;
    waitButton->setToolTip(i18n("Please wait..."));
    connect( waitButton, SIGNAL(clicked(bool)), SLOT(slotCancel()) );
    waitButton->setIcons("process-working-kde");
    waitButton->start();

    QVBoxLayout *ly = new QVBoxLayout(d->waitFrame);
    ly->setSpacing(0);
    ly->setContentsMargins(0, 0, 0, 0);
    ly->addWidget(waitButton);

    d->waitFrame->move(pos - QPoint(15, 15));
    d->waitFrame->show();
}

void TwitterApiWhoisWidget::checkAnchor( const QUrl url )
{
    kDebug();
    if(url.scheme()=="choqok"){
        if(url.host()=="close"){
            this->close();
        } else if (url.host() == "subscribe") {
            d->mBlog->createFriendship(d->currentAccount, d->username);
            connect(d->mBlog, SIGNAL(friendshipCreated(Choqok::Account*,QString)),
                    SLOT(slotFriendshipCreated(Choqok::Account*,QString)));
        } else if (url.host() == "unsubscribe") {
            d->mBlog->destroyFriendship(d->currentAccount, d->username);
            connect(d->mBlog, SIGNAL(friendshipDestroyed(Choqok::Account*,QString)),
                    SLOT(slotFriendshipDestroyed(Choqok::Account*,QString)));
        } else if (url.host() == "block") {
            d->mBlog->blockUser(d->currentAccount, d->username);
//             connect(d->mBlog, SIGNAL(userBlocked(Choqok::Account*,QString)), SLOT(slotUserBlocked(Choqok::Account*,QString)));
        }
    } else {
            Choqok::openUrl(url);
            close();
        }
}

void TwitterApiWhoisWidget::setupUi()
{
    kDebug();
    d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("icon://close"),
                            KIcon("dialog-close").pixmap(16) );


    QString style = "color: %1; background-color: %2";
    if ( Choqok::AppearanceSettings::isCustomUi() ) {
        setStyleSheet( style.arg(Choqok::AppearanceSettings::readForeColor().name())
                            .arg(Choqok::AppearanceSettings::readBackColor().name()) );
    } else {
        QPalette p = window()->palette();
        setStyleSheet( style.arg(p.color(QPalette::WindowText).name()).arg(p.color(QPalette::Window).name()) );
    }
}

void TwitterApiWhoisWidget::slotCancel()
{
    kDebug();
    if(d->waitFrame)
        d->waitFrame->deleteLater();
    if(d->job)
        d->job->kill();
    this->close();
}

void TwitterApiWhoisWidget::setActionImages()
{
    d->imgActions.clear();
    if(d->username.compare(d->currentAccount->username(), Qt::CaseInsensitive) != 0){
        if( d->currentAccount->friendsList().contains(d->username, Qt::CaseInsensitive) ){
            d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("icon://unsubscribe"),
                            KIcon("list-remove-user").pixmap(16) );
            d->imgActions += "<a href='choqok://unsubscribe'><img src='icon://unsubscribe' title='"+
                            i18n("Unsubscribe") +"'></a> ";
        } else {
            d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("icon://subscribe"),
                            KIcon("list-add-user").pixmap(16) );
            d->imgActions += "<a href='choqok://subscribe'><img src='icon://subscribe' title='"+
                            i18n("Subscribe") +"'></a> ";
        }

        d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("icon://block"),
                                         KIcon("dialog-cancel").pixmap(16) );
        d->imgActions += "<a href='choqok://block'><img src='icon://block' title='"+ i18n("Block") +"'></a>";
    }
}

void TwitterApiWhoisWidget::slotFriendshipCreated(Choqok::Account* theAccount, const QString &username)
{
    if(theAccount == d->currentAccount && username == d->username){
        setActionImages();
        updateHtml();
    }
}

void TwitterApiWhoisWidget::slotFriendshipDestroyed(Choqok::Account* theAccount, const QString &username)
{
    if(theAccount == d->currentAccount && username == d->username){
        setActionImages();
        updateHtml();
    }
}

// void TwitterApiWhoisWidget::slotUserBlocked(Choqok::Account* theAccount, const QString &username)
// {
//     if(theAccount == d->currentAccount && username == d->username){
//         Choqok::NotifyManager::success( i18n("Your posts are blocked for %1.", username) );
//     }
// }

#include "twitterapiwhoiswidget.moc"
