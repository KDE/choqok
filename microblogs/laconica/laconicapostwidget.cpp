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
#include <KPushButton>
#include <choqoktools.h>

const QRegExp LaconicaPostWidget::mGroupRegExp( "([\\s]|^)!([a-z0-9]+){1,64}",  Qt::CaseInsensitive );
const QRegExp LaconicaPostWidget::mLaconicaUserRegExp( "([\\s\\W]|^)@([a-z0-9]+){1,64}(?!(@))", Qt::CaseInsensitive );
const QRegExp LaconicaPostWidget::mLaconicaHashRegExp( "([\\s]|^)#([\\w_\\.\\-]+)", Qt::CaseInsensitive );

const QString subdomains = "(([a-z0-9-_]\\.)?)";
const QString dname = "(([a-z0-9-\\x0080-\\xFFFF]){1,63}\\.)+";
const QString zone ("((a[cdefgilmnoqrstuwxz])|(b[abdefghijlmnorstvwyz])|(c[acdfghiklmnoruvxyz])|(d[ejkmoz])|(e[ceghrstu])|\
(f[ijkmor])|(g[abdefghilmnpqrstuwy])|(h[kmnrtu])|(i[delmnoqrst])|(j[emop])|(k[eghimnprwyz])|(l[abcikrstuvy])|\
(m[acdefghklmnopqrstuvwxyz])|(n[acefgilopruz])|(om)|(p[aefghklnrstwy])|(qa)|(r[eosuw])|(s[abcdeghijklmnortuvyz])|\
(t[cdfghjkmnoprtvwz])|(u[agksyz])|(v[aceginu])|(w[fs])|(ye)|(z[amrw])\
|(asia|com|info|net|org|biz|name|pro|aero|cat|coop|edu|jobs|mobi|museum|tel|travel|gov|int|mil|local)|(中国)|(公司)|(网络)|(صر)|(امارات)|(рф))");
const QString domain = '(' + subdomains + dname + zone + ')';
const QRegExp LaconicaPostWidget::mStatusNetUserRegExp( "([\\s\\W]|^)@(([a-z0-9]+){1,64}@" + domain + ')', Qt::CaseInsensitive );

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

void LaconicaPostWidget::initUi()
{
    TwitterApiPostWidget::initUi();

    KPushButton *btn = buttons().value("btnResend");

    if(btn){
        QMenu *menu = new QMenu(btn);
        QAction *resend = new QAction(i18n("Manual ReSend"), menu);
        connect( resend, SIGNAL(triggered(bool)), SLOT(slotResendPost()) );
        QAction *repeat = new QAction(i18n("Repeat"), menu);
        repeat->setToolTip(i18n("Repeat post using API"));
        connect( repeat, SIGNAL(triggered(bool)), SLOT(repeatPost()) );
        menu->addAction(repeat);
        menu->addAction(resend);
        btn->setMenu(menu);
    }
}

LaconicaPostWidget::~LaconicaPostWidget()
{
    delete d;
}

QString LaconicaPostWidget::prepareStatus(const QString& text)
{
    QString res = TwitterApiPostWidget::prepareStatus(text);
    res.replace(mStatusNetUserRegExp,"\\1@<a href='user://\\2'>\\2</a>");  
    res.replace(mLaconicaUserRegExp,"\\1@<a href='user://\\2'>\\2</a>");
    res.replace(mGroupRegExp,"\\1!<a href='group://\\2'>\\2</a>");
    res.replace(mLaconicaHashRegExp,"\\1#<a href='tag://\\2'>\\2</a>");
    
    return res;
}

QString LaconicaPostWidget::generateSign()
{
    QString s = TwitterApiPostWidget::generateSign();
    if( !currentPost().title.isEmpty() ){
        s += i18n("<a href='%1'>in context</a>", currentPost().title);
    }
    return s;
}

void LaconicaPostWidget::checkAnchor(const QUrl& url)
{
    QString scheme = url.scheme();
    QAction * ret;
    if( scheme == "tag" ) {
        QString unpcode = QUrl::fromPunycode(url.host().toUtf8());
        unpcode.remove('.');
        unpcode.remove('-');
        unpcode.remove('_');
        
        KMenu menu;
        KAction *search = new KAction(KIcon("system-search"),
                                      i18n("Search for %1", unpcode), &menu);
        KAction *openInBrowser = new KAction(KIcon("applications-internet"),
                                             i18n("Open tag page in browser"), &menu);
        menu.addAction(search);
        menu.addAction(openInBrowser);
        menu.setDefaultAction(search);
        ret = menu.exec(QCursor::pos());
        if(ret == search){
            d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                                                        unpcode,
                                                        LaconicaSearch::ReferenceHashtag);
        } else if(ret == openInBrowser){
            Choqok::openUrl(QUrl(QString(d->account->homepageUrl().prettyUrl(KUrl::RemoveTrailingSlash)) +
                                  "tag/" + unpcode));
        }
    } else if( scheme == "group" ) {
        KMenu menu;
        KAction *search = new KAction(KIcon("system-search"),
                                      i18n("Show latest group posts", url.host()), &menu);
        KAction *openInBrowser = new KAction(KIcon("applications-internet"),
                                             i18n("Open group page in browser"), &menu);
        menu.addAction(search);
        menu.addAction(openInBrowser);
        menu.setDefaultAction(search);
        ret = menu.exec(QCursor::pos());
        if(ret == search){
            d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                                                        url.host(),
                                                        LaconicaSearch::ReferenceGroup);
        } else if(ret == openInBrowser){
            Choqok::openUrl(QUrl(QString(d->account->homepageUrl().prettyUrl(KUrl::RemoveTrailingSlash)) +
                                  "group/" + url.host()));
        }
    } else if(scheme == "user") {
        KMenu menu;
        KAction * info = new KAction( KIcon("user-identity"), i18nc("Who is user", "Who is %1",
                                                                    url.host()), &menu );
        KAction * from = new KAction(KIcon("edit-find-user"), i18nc("Posts from user", "Posts from %1",
                                                                    url.host()), &menu);
        KAction * to = new KAction(KIcon("meeting-attending"), i18nc("Replies to user", "Replies to %1",
                                                                     url.host()), &menu);
        KAction * openInBrowser = new KAction(KIcon("applications-internet"),
                                              i18nc("Open profile page in browser",
                                                    "Open profile in browser"), &menu);
        menu.addAction(info);
        if(currentPost().source != "ostatus") {
            menu.addAction(from);
            menu.addAction(to);
            from->setData(LaconicaSearch::FromUser);
            to->setData(LaconicaSearch::ToUser);
        }
        menu.addAction(openInBrowser);

        //Subscribe/UnSubscribe/Block
        bool hasBlock = false, isSubscribe = false;
        QString accountUsername = d->account->username().toLower();
        QString postUsername = url.host().toLower();
        KAction *subscribe = 0, *block = 0, *replyTo = 0, *dMessage = 0;
        if(accountUsername != postUsername){
            menu.addSeparator();
            QMenu *actionsMenu = menu.addMenu(KIcon("applications-system"), i18n("Actions"));
            replyTo = new KAction(KIcon("edit-undo"), i18nc("Write a message to user attention", "Write to %1",
                                                          url.host()), actionsMenu);
            actionsMenu->addAction(replyTo);
            if( d->account->friendsList().contains( url.host() ) ){
                dMessage = new KAction(KIcon("mail-message-new"), i18nc("Send direct message to user",
                                                                        "Send private message to %1",
                                                                        url.host()), actionsMenu);
                actionsMenu->addAction(dMessage);
                isSubscribe = false;//It's UnSubscribe
                subscribe = new KAction( KIcon("list-remove-user"),
                                         i18nc("Unsubscribe from user",
                                               "Unsubscribe from %1", url.host()), actionsMenu);
            } else {
                isSubscribe = true;
                subscribe = new KAction( KIcon("list-add-user"),
                                         i18nc("Subscribe to user",
                                               "Subscribe to %1", url.host()), actionsMenu);
            }
            hasBlock = true;
            block = new KAction( KIcon("dialog-cancel"),
                                 i18nc("Block user",
                                       "Block %1", url.host()), actionsMenu);
            if(currentPost().source != "ostatus") {
                actionsMenu->addAction(subscribe);
                actionsMenu->addAction(block);
            }
        }
        ret = menu.exec(QCursor::pos());
        if(ret == 0)
            return;
        if(ret == info) {
            TwitterApiWhoisWidget *wd = new TwitterApiWhoisWidget(d->account, url.host(), currentPost(), this);
            wd->show(QCursor::pos());
            return;
        } else if(ret == subscribe){
            if(isSubscribe) {
                d->mBlog->createFriendship(d->account, url.host());
            } else {
                d->mBlog->destroyFriendship(d->account, url.host());
            }
            return;
        } else if(ret == block){
            d->mBlog->blockUser(d->account, url.host());
            return;
        } else if(ret == openInBrowser){
            if( currentPost().author.homePageUrl.isEmpty() ) {
                Choqok::openUrl( QUrl( currentAccount()->microblog()->profileUrl(currentAccount(), url.host()) ) );
            } else {
                Choqok::openUrl( QUrl( currentPost().author.homePageUrl ) );
            }
            return;
        } else if(ret == replyTo){
            emit reply( QString("@%1").arg(url.host()), QString() );
            return;
        } else if(ret == dMessage){
            d->mBlog->showDirectMessageDialog( d->account, url.host() );
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
