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

#include "twitterpostwidget.h"
#include <twitterapihelper/twitterapimicroblog.h>
#include "twittersearch.h"
#include <KAction>
#include <KMenu>
#include <klocalizedstring.h>
#include <twitterapihelper/twitterapiwhoiswidget.h>
#include <twitterapihelper/twitterapiaccount.h>
#include <KPushButton>

TwitterPostWidget::TwitterPostWidget(Choqok::Account* account, const Choqok::Post& post, QWidget* parent): TwitterApiPostWidget(account, post, parent)
{

}

void TwitterPostWidget::initUi()
{
    TwitterApiPostWidget::initUi();

    KPushButton *btn = buttons().value("btnResend");

    if(btn){
        QMenu *menu = new QMenu(btn);
        QAction *resend = new QAction(i18n("Manual ReSend"), menu);
        connect( resend, SIGNAL(triggered(bool)), SLOT(slotResendPost()) );
        QAction *repeat = new QAction(i18n("Retweet"), menu);
        repeat->setToolTip(i18n("Retweet post using API"));
        connect( repeat, SIGNAL(triggered(bool)), SLOT(repeatPost()) );
        menu->addAction(repeat);
        menu->addAction(resend);
        btn->setMenu(menu);
    }
}

QString TwitterPostWidget::prepareStatus(const QString& text)
{
    QString res = TwitterApiPostWidget::prepareStatus(text);
    res.replace(mHashtagRegExp,"\\1#<a href='tag://\\2'>\\2</a>");
    return res;
}

void TwitterPostWidget::checkAnchor(const QUrl& url)
{
    QString scheme = url.scheme();
    TwitterApiMicroBlog* blog = qobject_cast<TwitterApiMicroBlog*>(currentAccount()->microblog());
    if( scheme == "tag" ) {
        blog->searchBackend()->requestSearchResults(currentAccount(),
                                                    url.host(),
                                                    (int)TwitterSearch::ReferenceHashtag);
    } else if(scheme == "user") {
        KMenu menu;
        KAction * info = new KAction( KIcon("user-identity"), i18nc("Who is user", "Who is %1", url.host()),
                                      &menu );
        KAction * from = new KAction(KIcon("edit-find-user"), i18nc("Posts from user", "Posts from %1",url.host()),
                                     &menu);
        KAction * to = new KAction(KIcon("meeting-attending"), i18nc("Replies to user", "Replies to %1",
                                                                     url.host()),
                                   &menu);
        KAction *cont = new KAction(KIcon("user-properties"),i18nc("Including user name", "Including %1",
                                                                   url.host()),
                                    &menu);
        from->setData(TwitterSearch::FromUser);
        to->setData(TwitterSearch::ToUser);
        cont->setData(TwitterSearch::ReferenceUser);
        menu.addAction(info);
        menu.addAction(from);
        menu.addAction(to);
        menu.addAction(cont);

        //Subscribe/UnSubscribe/Block
        bool isSubscribe = false;
        QString accountUsername = currentAccount()->username().toLower();
        QString postUsername = url.host().toLower();
        KAction *subscribe = 0, *block = 0 ;
        if(accountUsername != postUsername){
            menu.addSeparator();
            if( qobject_cast<TwitterApiAccount*>(currentAccount())->friendsList().contains( url.host(), Qt::CaseInsensitive ) ){
                isSubscribe = false;//It's UnSubscribe
                subscribe = new KAction( KIcon("list-remove-user"),
                                         i18nc("Unfollow user",
                                               "Unfollow %1", url.host()), &menu);
            } else {
                isSubscribe = true;
                subscribe = new KAction( KIcon("list-add-user"),
                                         i18nc("Follow user",
                                               "Follow %1", url.host()), &menu);
            }
            block = new KAction( KIcon("dialog-cancel"),
                                 i18nc("Block user",
                                       "Block %1", url.host()), &menu);
                                       menu.addAction(subscribe);
                                       menu.addAction(block);
        }

        QAction * ret = menu.exec(QCursor::pos());
        if(ret == 0)
            return;
        if(ret == info) {
            TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *>(currentAccount());
            TwitterApiWhoisWidget *wd = new TwitterApiWhoisWidget(acc, url.host(), this);
            wd->show(QCursor::pos());
            return;
        }
        if(ret == subscribe){
            
            if(isSubscribe) {
                blog->createFriendship(currentAccount(), url.host());
            } else {
                blog->destroyFriendship(currentAccount(), url.host());
            }
            return;
        }
        if(ret == block){
            blog->blockUser(currentAccount(), url.host());
            return;
        }
        int type = ret->data().toInt();
        blog->searchBackend()->requestSearchResults(currentAccount(),
                                                    url.host(),
                                                    type);
    } else
        TwitterApiPostWidget::checkAnchor(url);
}

