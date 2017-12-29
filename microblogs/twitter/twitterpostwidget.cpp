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

#include "twitterpostwidget.h"

#include <QAction>
#include <QMenu>
#include <QPushButton>

#include <KLocalizedString>

#include "choqokbehaviorsettings.h"
#include "choqoktools.h"
#include "mediamanager.h"
#include "textbrowser.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"
#include "twitterapiwhoiswidget.h"

#include "twittersearch.h"

const QRegExp TwitterPostWidget::mTwitterUserRegExp(QLatin1String("([\\s\\W]|^)@([a-z0-9_]+){1,20}"), Qt::CaseInsensitive);
const QRegExp TwitterPostWidget::mTwitterTagRegExp(QLatin1String("([\\s]|^)#([\\w_\\.\\-]+)"), Qt::CaseInsensitive);
const QString TwitterPostWidget::mQuotedTextBase(QLatin1String("<div style=\"padding-top:5px;padding-bottom:3px;\"><table style=\"%4 border-style:groove;\" border=\"1\"><tr><td style=\"padding-top:4px;padding-bottom:4px;padding-left:3px;padding-right:3px;\"><table> <tr><td width=\"40\" style=\"padding-right:5px;\"><img src=\"img://quotedProfileImage\" width=\"40\" height=\"40\" /></td><td dir=\"%2\"><p><b><a>%3:</b>&nbsp;%1</p></td></tr></table></td></tr></table></div>"));
const QUrl TwitterPostWidget::mQuotedAvatarResourceUrl(QLatin1String("img://quotedProfileImage"));
TwitterPostWidget::TwitterPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent): TwitterApiPostWidget(account, post, parent)
{
}

TwitterPostWidget::~TwitterPostWidget()
{
}

void TwitterPostWidget::initUi()
{
    TwitterApiPostWidget::initUi();
    
    if ( ! currentPost()->quotedPost.content.isEmpty() ) {
        if( !setupQuotedAvatar() ){
            _mainWidget->document()->addResource(QTextDocument::ImageResource, mQuotedAvatarResourceUrl,
                                                 Choqok::MediaManager::self()->defaultImage());
        }
        
        auto dir = getDirection(currentPost()->quotedPost.content);
        auto text = prepareStatus(currentPost()->quotedPost.content);
        QString user = QStringLiteral("<a href='user://%1'>%1</a>").arg(currentPost()->quotedPost.user.userName);
        QString quoteText = mQuotedTextBase.arg(text, dir, user, QLatin1String("background-color:%1;"));
        setExtraContents(quoteText.arg(getBackgroundColor()));
        updateUi();
    }
    
    QPushButton *btn = buttons().value(QLatin1String("btnResend"));

    if (btn) {
        QMenu *menu = new QMenu(btn);
        QAction *resend = new QAction(i18n("Manual ReSend"), menu);
        connect(resend, &QAction::triggered, this, &TwitterPostWidget::slotResendPost);
        QAction *repeat = new QAction(i18n("Retweet"), menu);
        repeat->setToolTip(i18n("Retweet post using API"));
        connect(repeat, &QAction::triggered, this, &TwitterPostWidget::repeatPost);
        // If person protects their acc, we will use simple adding RT before message
        if (!currentPost()->author.isProtected) {
            menu->addAction(repeat);
        }
        menu->addAction(resend);
        btn->setMenu(menu);
    }
}

QString TwitterPostWidget::prepareStatus(const QString &text)
{
    QString res = TwitterApiPostWidget::prepareStatus(text);
    res.replace(mTwitterUserRegExp, QLatin1String("\\1@<a href='user://\\2'>\\2</a>"));
    res.replace(mTwitterTagRegExp, QLatin1String("\\1#<a href='tag://\\2'>\\2</a>"));
    return res;
}

bool TwitterPostWidget::isRemoveAvailable()
{
    if (currentAccount()->username().compare(currentPost()->author.userName, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (currentPost()->isPrivate) {
        return true;
    } else {
        return false;
    }
}

void TwitterPostWidget::slotReplyToAll()
{
    QStringList nicks;
    nicks.append(currentPost()->author.userName);

    QString txt = QStringLiteral("@%1 ").arg(currentPost()->author.userName);

    int pos = 0;
    while ((pos = mTwitterUserRegExp.indexIn(currentPost()->content, pos)) != -1) {
        if (mTwitterUserRegExp.cap(2).toLower() != currentAccount()->username() &&
                mTwitterUserRegExp.cap(2).toLower() != currentPost()->author.userName &&
                !nicks.contains(mTwitterUserRegExp.cap(2).toLower())) {
            nicks.append(mTwitterUserRegExp.cap(2));
            txt += QStringLiteral("@%1 ").arg(mTwitterUserRegExp.cap(2));
        }
        pos += mTwitterUserRegExp.matchedLength();
    }

    txt.chop(1);

    Q_EMIT reply(txt, currentPost()->postId, currentPost()->author.userName);
}

void TwitterPostWidget::checkAnchor(const QUrl &url)
{
    QString scheme = url.scheme();
    TwitterApiMicroBlog *blog = qobject_cast<TwitterApiMicroBlog *>(currentAccount()->microblog());
    TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(currentAccount());
    if (scheme == QLatin1String("tag")) {
        blog->searchBackend()->requestSearchResults(currentAccount(),
                QUrl::fromAce(url.host().toUtf8()),
                (int)TwitterSearch::ReferenceHashtag);
    } else if (scheme == QLatin1String("user")) {
        QMenu menu;
        QAction *info = new QAction(QIcon::fromTheme(QLatin1String("user-identity")), i18nc("Who is user", "Who is %1", url.host()),
                                    &menu);
        QAction *from = new QAction(QIcon::fromTheme(QLatin1String("edit-find-user")), i18nc("Posts from user", "Posts from %1", url.host()),
                                    &menu);
        QAction *to = new QAction(QIcon::fromTheme(QLatin1String("meeting-attending")), i18nc("Replies to user", "Replies to %1",
                                  url.host()),
                                  &menu);
        QAction *cont = new QAction(QIcon::fromTheme(QLatin1String("user-properties")), i18nc("Including user name", "Including %1",
                                    url.host()),
                                    &menu);
        QAction *openInBrowser = new QAction(QIcon::fromTheme(QLatin1String("applications-internet")),
                                             i18nc("Open profile page in browser",
                                                     "Open profile in browser"), &menu);
        from->setData(TwitterSearch::FromUser);
        to->setData(TwitterSearch::ToUser);
        cont->setData(TwitterSearch::ReferenceUser);
        menu.addAction(info);
        menu.addAction(from);
        menu.addAction(to);
        menu.addAction(cont);
        menu.addAction(openInBrowser);

        //Subscribe/UnSubscribe/Block
        bool isSubscribe = false;
        QString accountUsername = currentAccount()->username().toLower();
        QString postUsername = url.host().toLower();
        QAction *subscribe = 0, *block = 0, *replyTo = 0, *dMessage = 0, *reportSpam = 0;
        if (accountUsername != postUsername) {
            menu.addSeparator();
            QMenu *actionsMenu = menu.addMenu(QIcon::fromTheme(QLatin1String("applications-system")), i18n("Actions"));
            replyTo = new QAction(QIcon::fromTheme(QLatin1String("edit-undo")), i18nc("Write a message to user attention", "Write to %1",
                                  url.host()), actionsMenu);
            actionsMenu->addAction(replyTo);
            if (account->friendsList().contains(url.host(),
                                                Qt::CaseInsensitive)) {
                dMessage = new QAction(QIcon::fromTheme(QLatin1String("mail-message-new")), i18nc("Send direct message to user",
                                       "Send private message to %1",
                                       url.host()), actionsMenu);
                actionsMenu->addAction(dMessage);
                isSubscribe = false;//It's UnSubscribe
                subscribe = new QAction(QIcon::fromTheme(QLatin1String("list-remove-user")),
                                        i18nc("Unfollow user",
                                              "Unfollow %1", url.host()), actionsMenu);
            } else {
                isSubscribe = true;
                subscribe = new QAction(QIcon::fromTheme(QLatin1String("list-add-user")),
                                        i18nc("Follow user",
                                              "Follow %1", url.host()), actionsMenu);
            }
            block = new QAction(QIcon::fromTheme(QLatin1String("dialog-cancel")),
                                i18nc("Block user",
                                      "Block %1", url.host()), actionsMenu);
            reportSpam = new QAction(QIcon::fromTheme(QLatin1String("irc-voice")),
                                     i18nc("Report user",
                                           "Report %1 as spam", url.host()), actionsMenu);

            actionsMenu->addAction(subscribe);
            actionsMenu->addAction(block);
            actionsMenu->addAction(reportSpam);
        }

        QAction *ret = menu.exec(QCursor::pos());
        if (ret == 0) {
            return;
        }
        if (ret == info) {
            TwitterApiWhoisWidget *wd = new TwitterApiWhoisWidget(account, url.host(),  *currentPost(), this);
            wd->show(QCursor::pos());
            return;
        } else if (ret == subscribe) {
            if (isSubscribe) {
                blog->createFriendship(currentAccount(), url.host());
            } else {
                blog->destroyFriendship(currentAccount(), url.host());
            }
            return;
        } else if (ret == block) {
            blog->blockUser(currentAccount(), url.host());
            return;
        } else if (ret == reportSpam) {
            blog->reportUserAsSpam(currentAccount(), url.host());
            return;
        } else if (ret == openInBrowser) {
            Choqok::openUrl(currentAccount()->microblog()->profileUrl(currentAccount(), url.host()));
            return;
        } else if (ret == replyTo) {
            Q_EMIT reply(QStringLiteral("@%1").arg(url.host()), QString(), url.host());
            return;
        } else if (ret == dMessage) {
            blog->showDirectMessageDialog(account, url.host());
            return;
        }
        int type = ret->data().toInt();
        blog->searchBackend()->requestSearchResults(currentAccount(),
                url.host(), type, QString(), Choqok::BehaviorSettings::countOfPosts());
    } else {
        TwitterApiPostWidget::checkAnchor(url);
    }
}

bool TwitterPostWidget::setupQuotedAvatar()
{
    QPixmap pix = Choqok::MediaManager::self()->fetchImage(currentPost()->quotedPost.user.profileImageUrl,
                                                           Choqok::MediaManager::Async);
    if (!pix.isNull()) {
        quotedAvatarFetched(currentPost()->quotedPost.user.profileImageUrl, pix);
        return true;
    } else {
        connect(Choqok::MediaManager::self(), &Choqok::MediaManager::imageFetched,
                this, &TwitterPostWidget::quotedAvatarFetched);
        connect(Choqok::MediaManager::self(), &Choqok::MediaManager::fetchError,
                this, &TwitterPostWidget::quotedAvatarFetchError);
        return false;
    }
}

void TwitterPostWidget::quotedAvatarFetched(const QUrl &remoteUrl, const QPixmap &pixmap)
{
    if (remoteUrl == currentPost()->quotedPost.user.profileImageUrl) {
        _mainWidget->document()->addResource(QTextDocument::ImageResource, mQuotedAvatarResourceUrl, pixmap);
        disconnect(Choqok::MediaManager::self(), &Choqok::MediaManager::imageFetched,
                   this, &TwitterPostWidget::quotedAvatarFetched);
        disconnect(Choqok::MediaManager::self(), &Choqok::MediaManager::fetchError,
                   this, &TwitterPostWidget::quotedAvatarFetchError);
    }
}

void TwitterPostWidget::quotedAvatarFetchError(const QUrl &remoteUrl, const QString &errMsg)
{
    Q_UNUSED(errMsg);
    if (remoteUrl == currentPost()->quotedPost.user.profileImageUrl) {
        ///Avatar fetching is failed! but will not disconnect to get the img if it fetches later!
        _mainWidget->document()->addResource(QTextDocument::ImageResource, mQuotedAvatarResourceUrl,
                                             QIcon::fromTheme(QLatin1String("image-missing")).pixmap(40));
    }
}

QString TwitterPostWidget::getBackgroundColor()
{
    QString style = styleSheet();
    QLatin1String str{ "background-color:rgb(" };
    int idx = style.indexOf(str);
    if(idx != -1){
        idx += str.size();
        int endIdx = style.indexOf(QLatin1String(");"), idx);
        if( endIdx != -1 ){
            QStringList rgb = style.mid(idx, endIdx-idx).split(QLatin1Char(','));
            if( rgb.size() == 3 ){
                return QStringLiteral("#%1%2%3").arg( rgb[0].toInt() - 20, 2, 16, QLatin1Char('0') )
                                                .arg( rgb[1].toInt() - 20, 2, 16, QLatin1Char('0') )
                                                .arg( rgb[2].toInt() - 20, 2, 16, QLatin1Char('0') );
            }
        }
    }
    
    return QLatin1String("#ffffff");
}
