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

const QString TwitterApiWhoisWidget::baseText("\
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
</table>");

class TwitterApiWhoisWidget::Private
{
public:
    Private(TwitterApiAccount *account)
    :currentAccount(account), waitFrame(0), job(0)
    {}
    KTextBrowser *wid;
    TwitterApiAccount *currentAccount;
    QFrame *waitFrame;
    KJob *job;
    Choqok::Post currentPost;

    QString followersCount;
    QString friendsCount;
    QString timeZone;
    QString lockImg;
//     bool isFollowing;
};

TwitterApiWhoisWidget::TwitterApiWhoisWidget(TwitterApiAccount* theAccount, const QString& username,
                                             QWidget* parent)
    : QFrame(parent), d(new Private(theAccount))
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

    url.addPath("/users/show.xml");
    url.addQueryItem("screen_name", username);

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
        slotCancel();
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    QDomDocument doc;
    doc.setContent(stj->data());

    QDomElement root = doc.documentElement();
    if ( root.tagName() != "user" ) {
        kDebug()<<"There's no user tag in returned document from server! Date is:\n\t"<<stj->data();
        d->wid->setText(i18n("Sorry, Cannot Load user information!"));
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
        } else if( elm.tagName() == "protected" ){
            if(elm.text() == "true"){
                d->lockImg = "<img src=\"icon://lock\">";
                d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("icon://lock"),
                                                 KIcon("object-locked").pixmap(16) );
            }
        } else if(elm.tagName() == "status") {
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
    setHtml();
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
        setHtml();
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
        setHtml();
    }
}

void TwitterApiWhoisWidget::setHtml()
{
    kDebug();
    QString url = d->currentPost.author.homePageUrl.isEmpty() ? QString()
                : QString("<a title='%1' href='%1'>%1</a>").arg(d->currentPost.author.homePageUrl);

//     QString dir = post.content.isRightToLeft() ? "rtl" : "ltr";
    QString html = QString(baseText).arg(d->currentPost.author.userName)
                                    .arg(d->currentPost.author.realName)
                                    .arg(d->currentPost.author.location)
                                    .arg(d->timeZone)
                                    .arg(url)
                                    .arg(d->currentPost.author.description)
                                    .arg(d->currentPost.content)//.arg(dir);
                                    .arg(d->friendsCount)
                                    .arg(d->followersCount)
                                    .arg(d->lockImg);

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
        if(url.host()=="close")
            this->close();
//     else if (url.host() == "follow") {
//         }
    } else {
        KToolInvocation::invokeBrowser(url.toString());
        close();
    }
}

void TwitterApiWhoisWidget::setupUi()
{
    kDebug();
    d->wid->document()->addResource( QTextDocument::ImageResource, QUrl("icon://close"),
                            KIcon("dialog-close").pixmap(16) );
//     w->document()->addResource( QTextDocument::ImageResource, QUrl("icon://follow"),
//                             KIcon("list-add-user").pixmap(16) );

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

#include "twitterapiwhoiswidget.moc"
