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

#include "laconicapostwidget.h"
#include <twitterapihelper/twitterapiaccount.h>
#include <KDebug>
#include <twitterapihelper/twitterapimicroblog.h>
#include "laconicasearch.h"
#include <KMenu>
#include <KAction>
#include <klocalizedstring.h>
#include <twitterapihelper/twitterapiwhoiswidget.h>
#include <choqokbehaviorsettings.h>
#include <quickpost.h>
#include "laconicaaccount.h"
#include "laconicamicroblog.h"
#include <notifymanager.h>

const QRegExp LaconicaPostWidget::mGroupRegExp("([\\s]|^)!([^\\s\\W]+)");

class LaconicaPostWidget::Private
{
public:
    Private(Choqok::Account* theAccount)
    {
        account = qobject_cast<LaconicaAccount *>(theAccount);
        mBlog = qobject_cast<LaconicaMicroBlog *>(account->microblog());
    }
    LaconicaAccount *account;
    LaconicaMicroBlog *mBlog;
    QString tmpUsername;
};

LaconicaPostWidget::LaconicaPostWidget(Choqok::Account* account, const Choqok::Post& post, QWidget* parent)
: TwitterApiPostWidget(account, post, parent), d( new Private(account) )
{

}

LaconicaPostWidget::~LaconicaPostWidget()
{
    delete d;
}

QString LaconicaPostWidget::prepareStatus(const QString& text)
{
    QString res = TwitterApiPostWidget::prepareStatus(text);
    QString homepage = d->account->homepageUrl().prettyUrl(KUrl::RemoveTrailingSlash);
    res.replace(mGroupRegExp,"\\1!<a href='group://\\2'>\\2</a> <a href='"+ homepage +
    "/group/\\2'>"+ webIconText +"</a>");
    res.replace(mHashtagRegExp,"\\1#<a href='tag://\\2'>\\2</a> <a href='"+ homepage +
    "/tag/\\1'>"+ webIconText +"</a>");
    return res;
}

void LaconicaPostWidget::checkAnchor(const QUrl& url)
{
    QString scheme = url.scheme();
    if( scheme == "tag" ) {
        d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                                                    url.host(),
                                                    LaconicaSearch::ReferenceHashtag);
    } else if( scheme == "group" ) {
        d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                                                    url.host(),
                                                    LaconicaSearch::ReferenceGroup);
    } else if(scheme == "user") {
        KMenu menu;
        KAction * info = new KAction( KIcon("user-identity"), i18nc("Who is user", "Who is %1",
                                                                    url.host()), &menu );
        KAction * from = new KAction(KIcon("edit-find-user"), i18nc("Posts from user", "Posts from %1",
                                                                    url.host()), &menu);
        KAction * to = new KAction(KIcon("meeting-attending"), i18nc("Replies to user", "Replies to %1",
                                                                     url.host()), &menu);
        menu.addAction(info);
        menu.addAction(from);
        menu.addAction(to);
        from->setData(LaconicaSearch::FromUser);
        to->setData(LaconicaSearch::ToUser);
        QAction * ret;

        //Subscribe/UnSubscribe/Block
        bool hasBlock = false, isSubscribe = false;
        QString accountUsername = d->account->username().toLower();
        QString postUsername = url.host().toLower();
        KAction *subscribe = 0, *block = 0 ;
        if(accountUsername != postUsername){
            menu.addSeparator();
            if( d->account->friendsList().contains( url.host() ) ){
                isSubscribe = false;//It's UnSubscribe
                subscribe = new KAction( KIcon("list-remove-user"),
                                         i18nc("Unsubscribe from user",
                                               "Unsubscribe from %1", url.host()), &menu);
            } else {
                isSubscribe = true;
                subscribe = new KAction( KIcon("list-add-user"),
                                         i18nc("Subscribe to user",
                                               "Subscribe to %1", url.host()), &menu);
            }
            hasBlock = true;
            block = new KAction( KIcon("dialog-cancel"),
                                 i18nc("Block user",
                                       "Block %1", url.host()), &menu);
            menu.addAction(subscribe);
            menu.addAction(block);
        }
        ret = menu.exec(QCursor::pos());
        if(ret == 0)
            return;
        if(ret == info) {
            TwitterApiWhoisWidget *wd = new TwitterApiWhoisWidget(d->account, url.host(), this);
            wd->show(QCursor::pos());
            return;
        }
        if(ret == subscribe){
            if(isSubscribe) {
                d->mBlog->createFriendship(d->account, url.host());
            } else {
                d->mBlog->destroyFriendship(d->account, url.host());
            }
            return;
        }
        if(ret == block){
            d->mBlog->blockUser(d->account, url.host());
            return;
        }
        int type = ret->data().toInt();
        d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                                                    url.host(),
                                                    type);
    } else
        TwitterApiPostWidget::checkAnchor(url);
}

void LaconicaPostWidget::slotResendPost()
{
    QString text = generateResendText();

    if(d->account->isChangeExclamationMark()){
        int index = 0;
        while( true ){
            index = mGroupRegExp.indexIn(text, index);
            if(index != -1)
                text.replace( index+1, 1, d->account->changeExclamationMarkToText());
            else
                break;
        }
    }

    if( (Choqok::BehaviorSettings::resendWithQuickPost() || currentAccount()->isReadOnly()) &&
        Choqok::UI::Global::quickPostWidget() )
        Choqok::UI::Global::quickPostWidget()->setText(text);
    else
        emit resendPost(text);
}
