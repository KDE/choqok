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

#include "twitterapipostwidget.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

#include "twitterapidebug.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <KPushButton>

#include "choqokappearancesettings.h"
#include "mediamanager.h"
#include "microblog.h"
#include "textbrowser.h"
#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"
#include "twitterapishowthread.h"

const QIcon TwitterApiPostWidget::unFavIcon(Choqok::MediaManager::convertToGrayScale(QIcon::fromTheme("rating").pixmap(16)) );

class TwitterApiPostWidget::Private
{
public:
    Private(Choqok::Account* account)
        :isBasePostShowed(false)
    {
        mBlog = qobject_cast<TwitterApiMicroBlog*>( account->microblog() );
    }
    KPushButton *btnFav;
    bool isBasePostShowed;
    TwitterApiMicroBlog *mBlog;
};

TwitterApiPostWidget::TwitterApiPostWidget(Choqok::Account* account, Choqok::Post *post, QWidget* parent)
    : PostWidget(account, post, parent), d(new Private(account))
{
    mainWidget()->document()->addResource( QTextDocument::ImageResource, QUrl("icon://thread"),
                             QIcon::fromTheme("go-top").pixmap(10) );
}

TwitterApiPostWidget::~TwitterApiPostWidget()
{
    delete d;
}

void TwitterApiPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    KPushButton *btnRe = addButton( "btnReply",i18nc( "@info:tooltip", "Reply" ), "edit-undo" );
    QMenu *menu = new QMenu(btnRe);

    QAction *actRep = new QAction(QIcon::fromTheme("edit-undo"), i18n("Reply to %1", currentPost()->author.userName), menu);
    menu->addAction(actRep);
    connect( actRep, SIGNAL(triggered(bool)), SLOT(slotReply()) );
    connect( btnRe, SIGNAL(clicked(bool)), SLOT(slotReply()) );

    QAction *actWrite = new QAction( QIcon::fromTheme("document-edit"), i18n("Write to %1", currentPost()->author.userName),
                                     menu );
    menu->addAction(actWrite);
    connect( actWrite, SIGNAL(triggered(bool)), SLOT(slotWriteTo()) );

    if( !currentPost()->isPrivate ) {
        QAction *actReplytoAll = new QAction(i18n("Reply to all"), menu);
        menu->addAction(actReplytoAll);
        connect( actReplytoAll, SIGNAL(triggered(bool)), SLOT(slotReplyToAll()) );
    }

    menu->setDefaultAction(actRep);
    btnRe->setDelayedMenu(menu);

    if( !currentPost()->isPrivate ) {
        d->btnFav = addButton( "btnFavorite",i18nc( "@info:tooltip", "Favorite" ), "rating" );
        d->btnFav->setCheckable(true);
        connect( d->btnFav, SIGNAL(clicked(bool)), SLOT(setFavorite()) );
        updateFavStat();
    }
}

QString TwitterApiPostWidget::prepareStatus(const QString& text)
{
    return Choqok::UI::PostWidget::prepareStatus(text);
}

QString TwitterApiPostWidget::generateSign()
{
    QString sign;
    QString profUrl = currentAccount()->microblog()->profileUrl(currentAccount(),
                                                                currentPost()->author.userName);
    sign = "<b><a href='user://"+currentPost()->author.userName+"' title=\"" +
    Qt::escape(currentPost()->author.description) + "\">" + currentPost()->author.userName +
    "</a> - </b>";
    //<img src=\"icon://web\" />
    sign += "<a href=\"" + currentPost()->link +
    "\" title=\"" + currentPost()->creationDateTime.toString( Qt::DefaultLocaleLongDate ) + "\">%1</a>";
    if ( currentPost()->isPrivate ) {
        if( currentPost()->replyToUserName.compare( currentAccount()->username(), Qt::CaseInsensitive ) == 0 ) {
            sign.prepend( "From " );
        } else {
            sign.prepend( "To " );
        }
    } else {
        if( !currentPost()->source.isNull() ) {
            sign += " - ";
            if(currentPost()->source == "ostatus" && !currentPost()->author.homePageUrl.isEmpty()) {
                QUrl srcUrl(currentPost()->author.homePageUrl);
                sign += i18n( "<a href='%1' title='Sent from %2 via OStatus'>%2</a>",
                              currentPost()->author.homePageUrl,
                              srcUrl.host());
            } else {
                sign += currentPost()->source;
            }
        }
        if ( !currentPost()->replyToPostId.isEmpty() ) {
            QString link = currentAccount()->microblog()->postUrl( currentAccount(), currentPost()->replyToUserName,
                                                                   currentPost()->replyToPostId );
            QString showConMsg = i18n("Show Conversation");
            QString threadlink;
            if( currentPost()->conversationId.isEmpty() )
                threadlink = "thread://" + currentPost()->postId;
            else
                threadlink = "conversation://" + currentPost()->conversationId;
            sign += " - " +
            i18n("<a href='replyto://%1'>in reply to</a> @<a href='user://%4'>%4</a>&nbsp;<a href=\"%2\" title=\"%2\">%3</a>",
                 currentPost()->replyToPostId, link, webIconText, currentPost()->replyToUserName) + ' ';
            sign += "<a title=\""+ showConMsg +"\" href=\"" + threadlink + "\"><img src=\"icon://thread\" /></a>";
        }
    }

    //ReTweet detection:
    if( !currentPost()->repeatedFromUsername.isEmpty() ){
        QString retweet;
        retweet += "<br/>"
                +  d->mBlog->generateRepeatedByUserTooltip( QString("<a href='user://%1'>%2</a>").arg( currentPost()->repeatedFromUsername).arg(currentPost()->repeatedFromUsername) );
        sign.append(retweet);
    }
    sign.prepend("<p dir='ltr'>");
    sign.append( "</p>" );
    return sign;
}

void TwitterApiPostWidget::slotReply()
{
    setReadWithSignal();
    if(currentPost()->isPrivate){
        TwitterApiAccount *account= qobject_cast<TwitterApiAccount*>( currentAccount() );
        d->mBlog->showDirectMessageDialog( account, currentPost()->author.userName );
    } else {
        QString replyto = QString("@%1").arg(currentPost()->author.userName);
        QString postId = currentPost()->postId;
        QString username = currentPost()->author.userName;
        if( !currentPost()->repeatedFromUsername.isEmpty() ){
            replyto.prepend(QString("@%1 ").arg(currentPost()->repeatedFromUsername));
            postId = currentPost()->repeatedPostId;
        }
        Q_EMIT reply( replyto, postId,  username);
    }
}

void TwitterApiPostWidget::slotWriteTo()
{
    Q_EMIT reply( QString("@%1").arg(currentPost()->author.userName), QString(), currentPost()->author.userName );
}

void TwitterApiPostWidget::slotReplyToAll()
{
    QString txt = QString("@%1").arg(currentPost()->author.userName);
    Q_EMIT reply(txt, currentPost()->postId, currentPost()->author.userName);
}

void TwitterApiPostWidget::setFavorite()
{
    setReadWithSignal();
    TwitterApiMicroBlog *mic = d->mBlog;
    if(currentPost()->isFavorited){
        connect(mic, SIGNAL(favoriteRemoved(Choqok::Account*,QString)),
                this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
        mic->removeFavorite(currentAccount(), currentPost()->postId);
    } else {
        connect(mic, SIGNAL(favoriteCreated(Choqok::Account*,QString)),
                   this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
        mic->createFavorite(currentAccount(), currentPost()->postId);
    }
}

void TwitterApiPostWidget::slotSetFavorite(Choqok::Account *theAccount, const QString& postId)
{
    if(currentAccount() == theAccount && postId == currentPost()->postId){
        qCDebug(CHOQOK)<<postId;
        currentPost()->isFavorited = !currentPost()->isFavorited;
        updateFavStat();
        disconnect(d->mBlog, SIGNAL(favoriteRemoved(Choqok::Account*,QString)),
                   this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
        disconnect(d->mBlog, SIGNAL(favoriteCreated(Choqok::Account*,QString)),
                   this, SLOT(slotSetFavorite(Choqok::Account*,QString)) );
    }
}

void TwitterApiPostWidget::updateFavStat()
{
    if(currentPost()->isFavorited){
        d->btnFav->setChecked(true);
        d->btnFav->setIcon(QIcon::fromTheme("rating"));
    } else {
        d->btnFav->setChecked(false);
        d->btnFav->setIcon(unFavIcon);
    }
}

void TwitterApiPostWidget::checkAnchor(const QUrl & url)
{
    QString scheme = url.scheme();
    if( scheme == "replyto" ) {
        if(d->isBasePostShowed) {
            setContent( prepareStatus(currentPost()->content).replace("<a href","<a style=\"text-decoration:none\" href",Qt::CaseInsensitive) );
            updateUi();
            d->isBasePostShowed = false;
            return;
        } else {
            connect(currentAccount()->microblog(), SIGNAL(postFetched(Choqok::Account*,Choqok::Post*)),
                    this, SLOT(slotBasePostFetched(Choqok::Account*,Choqok::Post*)) );
            Choqok::Post *ps = new Choqok::Post;
            ps->postId = url.host();
            currentAccount()->microblog()->fetchPost(currentAccount(), ps);
        }
    } else if (scheme == "thread") {
        TwitterApiShowThread *wd = new TwitterApiShowThread(currentAccount(), currentPost(), NULL);
        wd->resize(this->width(), wd->height());
        connect(wd, SIGNAL(forwardReply(QString,QString,QString)),
                this, SIGNAL(reply(QString,QString,QString)));
        connect(wd, SIGNAL(forwardResendPost(QString)),
                this, SIGNAL(resendPost(QString)));
        wd->show();
    } else {
        Choqok::UI::PostWidget::checkAnchor(url);
    }

}

void TwitterApiPostWidget::slotBasePostFetched(Choqok::Account* theAccount, Choqok::Post* post)
{
    if(theAccount == currentAccount() && post && post->postId == currentPost()->replyToPostId){
        qCDebug(CHOQOK);
        disconnect( currentAccount()->microblog(), SIGNAL(postFetched(Choqok::Account*,Choqok::Post*)),
                   this, SLOT(slotBasePostFetched(Choqok::Account*,Choqok::Post*)) );
        if(d->isBasePostShowed)
            return;
        d->isBasePostShowed = true;
        QString color;
        if( Choqok::AppearanceSettings::isCustomUi() ) {
            color = Choqok::AppearanceSettings::readForeColor().lighter().name();
        } else {
            color = this->palette().dark().color().name();
        }
        QString baseStatusText = "<p style=\"margin-top:10px; margin-bottom:10px; margin-left:20px;\
        margin-right:20px; text-indent:0px\"><span style=\" color:" + color + ";\">";
        baseStatusText += "<b><a href='user://"+ post->author.userName +"'>" +
        post->author.userName + "</a> :</b> ";

        baseStatusText += prepareStatus( post->content ) + "</p>";
        setContent( content().prepend( baseStatusText.replace("<a href","<a style=\"text-decoration:none\" href",Qt::CaseInsensitive) ) );
        updateUi();
//         delete post;
    }
}

void TwitterApiPostWidget::repeatPost()
{
    setReadWithSignal();
    QString postId;
    if(currentPost()->repeatedPostId.isEmpty())
        postId = currentPost()->postId;
    else
        postId = currentPost()->repeatedPostId;
    if( KMessageBox::questionYesNo(Choqok::UI::Global::mainWindow(), d->mBlog->repeatQuestion(),
                               QString(), KStandardGuiItem::yes(), KStandardGuiItem::cancel(),
                               "dontAskRepeatConfirm") == KMessageBox::Yes )
        d->mBlog->repeatPost(currentAccount(), postId);
}


