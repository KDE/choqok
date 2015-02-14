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

#include "twitterapiwhoiswidget.h"

#include <QDesktopWidget>
#include <QDomDocument>
#include <QPointer>
#include <QVBoxLayout>

#include <KAnimatedButton>
#include <KApplication>
#include <KDebug>
#include <KIcon>
#include <KIO/Job>
#include <KLocalizedString>
#include <KNotification>
#include <KProcess>
#include <KStatusBar>
#include <KTextBrowser>
#include <KToolInvocation>
#include <KUrl>

#include <qjson/parser.h>

#include "choqokappearancesettings.h"
#include "choqoktools.h"
#include "choqoktypes.h"
#include "mediamanager.h"
#include "microblog.h"
#include "notifymanager.h"
#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"

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
    QPointer<KJob> job;
    Choqok::Post currentPost;
    QString username;
    QString errorMessage;

    QString followersCount;
    QString friendsCount;
    QString statusesCount;
    QString timeZone;
    QString imgActions;
//     bool isFollowing;
};

TwitterApiWhoisWidget::TwitterApiWhoisWidget(TwitterApiAccount* theAccount, const QString& username,
                                             const Choqok::Post &post, QWidget* parent)
    : QFrame(parent), d(new Private(theAccount, username))
{
    kDebug();
    setAttribute(Qt::WA_DeleteOnClose);
    d->currentPost = post;
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
    QString urlStr;
    QString user = username;
    if(user.contains('@')) {
        QStringList lst = user.split('@');
        if(lst.count() == 2){//USER@HOST
            QString host = lst[1];
            urlStr = QString("http://%1/api").arg(host);
            user = lst[0];
        }
    } else if( d->currentPost.source == "ostatus" && !d->currentPost.author.homePageUrl.isEmpty() ) {
        urlStr = d->currentPost.author.homePageUrl;
        if(urlStr.endsWith(user)) {
            int len = urlStr.length();
            int userLen = user.length();
            urlStr.remove(len - userLen, userLen);
            kDebug()<<urlStr;
        }
        urlStr.append("api");
    } else {
        urlStr = theAccount->apiUrl().url();
    }
    KUrl url( urlStr );

    url.addPath( QString( "/users/show/%1.json" ).arg(user));

//     kDebug() << url;

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    if( d->currentPost.source != "ostatus" )
        job->addMetaData("customHTTPHeader", "Authorization: " + d->mBlog->authorizationHeader(theAccount,
                                                                                           url, QOAuth::GET));

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
//     kDebug()<<stj->data();
    QJson::Parser parser;
    bool ok;
    QVariantMap map = parser.parse(stj->data(), &ok).toMap();

    Choqok::Post post;
    if ( !ok ){
        kDebug()<<"JSON parsing failed! Data is:\n\t"<<stj->data();
        d->errorMessage = i18n("Cannot load user information.");
        updateHtml();
        showForm();
        return;
    }

    QString timeStr;
    d->errorMessage = map["error"].toString();
    if( d->errorMessage.isEmpty() ) { //No Error
        post.author.realName = map["name"].toString();
        post.author.userName = map["screen_name"].toString();
        post.author.location = map["location"].toString();
        post.author.description = map["description"].toString();
        post.author.profileImageUrl = map["profile_image_url"].toString();
        post.author.homePageUrl = map["url"].toString();
        d->timeZone = map["time_zone"].toString();
        d->followersCount = map["followers_count"].toString();
        d->friendsCount = map["friends_count"].toString();
        d->statusesCount = map["statuses_count"].toString();
        QVariantMap var = map["status"].toMap();
        post.content = var["text"].toString();
        post.creationDateTime = d->mBlog->dateFromString(var["created_at"].toString());
        post.isFavorited = var["favorited"].toBool();
        post.postId = var["id"].toString();
        post.replyToPostId = var["in_reply_to_status_id"].toString();
        post.replyToUserId = var["in_reply_to_user_id"].toString();
        post.replyToUserName = var["in_reply_to_screen_name"].toString();
        post.source = var["source"].toString();
        d->currentPost = post;
    }

    updateHtml();
    showForm();

    QPixmap userAvatar = Choqok::MediaManager::self()->fetchImage( post.author.profileImageUrl,
                                                                    Choqok::MediaManager::Async );

    if(!userAvatar.isNull()) {
        d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"),
                                         userAvatar );
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
    QString html;
    if( d->errorMessage.isEmpty() ) {
        QString url = d->currentPost.author.homePageUrl.isEmpty() ? QString()
                    : QString("<a title='%1' href='%1'>%1</a>").arg(d->currentPost.author.homePageUrl);

        QString mainTable = QString("<table width='100%'><tr>\
        <td width=49><img width=48 height=48 src='img://profileImage'/>\
        <center><table width='100%' cellpadding='3'><tr>%1</tr></table></center></td>\
        <td><table width='100%'><tr><td><font size=5><b>%2</b></font></td>\
        <td><a href='choqok://close'><img src='icon://close' title='" + i18n("Close") + "' align='right' /></a></td></tr></table><br/>\
        <b>@%3</b>&nbsp;<font size=3>%4</font><font size=2>%5</font><br/>\
        <i>%6</i><br/>\
        <font size=3>%7</font></td></tr></table>")
        .arg(d->imgActions)
        .arg(Qt::escape(d->currentPost.author.realName))
        .arg(d->currentPost.author.userName).arg(Qt::escape(d->currentPost.author.location))
        .arg(!d->timeZone.isEmpty() ? '(' + d->timeZone + ')' : QString())
        .arg(d->currentPost.author.description)
        .arg(url);

        QString countTable = QString("<table><tr>\
        <td><b>%1</b><br/>" + i18nc("User posts", "Posts") + "</td>\
        <td><b>%2</b><br/>" + i18nc("User friends", "Friends") + "</td>\
        <td><b>%3</b><br/>" + i18nc("User followers" , "Followers") + "</td>\
        </tr></table><br/>")
        .arg(d->statusesCount)
        .arg(d->friendsCount)
        .arg(d->followersCount);
        
        html = mainTable + countTable;
        
        if (!d->currentPost.content.isEmpty()){
            html.append(QString(i18n("<table><tr><b>Last Status:</b> %1</tr></table>")).arg(d->currentPost.content));
        }

    } else {
        html = i18n("<h3>%1</h3>", d->errorMessage);
    }
    d->wid->setHtml(html);
}

void TwitterApiWhoisWidget::showForm()
{
    kDebug();
    QPoint pos = d->waitFrame->pos();
    d->waitFrame->deleteLater();
    d->wid->resize(320, 200);
    d->wid->document()->setTextWidth(width()-2);
    d->wid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
