/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gnusocialapipostwidget.h"

#include <QAction>
#include <QMenu>
#include <QPushButton>

#include <KLocalizedString>

#include "choqokbehaviorsettings.h"
#include "choqoktools.h"
#include "notifymanager.h"
#include "quickpost.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"
#include "twitterapiwhoiswidget.h"

#include "gnusocialapiaccount.h"
#include "gnusocialapiconversationtimelinewidget.h"
#include "gnusocialapidebug.h"
#include "gnusocialapimicroblog.h"
#include "gnusocialapisearch.h"

const QRegExp GNUSocialApiPostWidget::mGroupRegExp(QLatin1String("([\\s]|^)!([a-z0-9]+){1,64}"),  Qt::CaseInsensitive);
const QRegExp GNUSocialApiPostWidget::mGNUSocialApiUserRegExp(QLatin1String("([\\s\\W]|^)@([a-z0-9_]+){1,64}(?!(@))"), Qt::CaseInsensitive);
const QRegExp GNUSocialApiPostWidget::mGNUSocialApiHashRegExp(QLatin1String("([\\s]|^)#([\\w_\\.\\-]+)"), Qt::CaseInsensitive);

const QString subdomains = QLatin1String("(([a-z0-9-_]\\.)?)");
const QString dname = QLatin1String("(([a-z0-9-\\x0080-\\xFFFF]){1,63}\\.)+");
const QString zone = QLatin1String("((a[cdefgilmnoqrstuwxz])|(b[abdefghijlmnorstvwyz])|(c[acdfghiklmnoruvxyz])|(d[ejkmoz])|(e[ceghrstu])|\
(f[ijkmor])|(g[abdefghilmnpqrstuwy])|(h[kmnrtu])|(i[delmnoqrst])|(j[emop])|(k[eghimnprwyz])|(l[abcikrstuvy])|\
(m[acdefghklmnopqrstuvwxyz])|(n[acefgilopruz])|(om)|(p[aefghklnrstwy])|(qa)|(r[eosuw])|(s[abcdeghijklmnortuvyz])|\
(t[cdfghjkmnoprtvwz])|(u[agksyz])|(v[aceginu])|(w[fs])|(ye)|(z[amrw])\
|(asia|com|info|net|org|biz|name|pro|aero|cat|coop|edu|jobs|mobi|museum|tel|travel|gov|int|mil|local)|(中国)|(公司)|(网络)|(صر)|(امارات)|(рф))");
const QString domain = QLatin1Char('(') + subdomains + dname + zone + QLatin1Char(')');
const QRegExp GNUSocialApiPostWidget::mStatusNetUserRegExp(QLatin1String("([\\s\\W]|^)@(([a-z0-9]+){1,64}@") + domain + QLatin1Char(')'), Qt::CaseInsensitive);

class GNUSocialApiPostWidget::Private
{
public:
    Private(Choqok::Account *theAccount)
    {
        account = qobject_cast<GNUSocialApiAccount *>(theAccount);
        mBlog = qobject_cast<GNUSocialApiMicroBlog *>(account->microblog());
    }
    GNUSocialApiAccount *account;
    GNUSocialApiMicroBlog *mBlog;
    QString tmpUsername;
};

GNUSocialApiPostWidget::GNUSocialApiPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent)
    : TwitterApiPostWidget(account, post, parent), d(new Private(account))
{

}

void GNUSocialApiPostWidget::initUi()
{
    TwitterApiPostWidget::initUi();

    QPushButton *btn = buttons().value(QLatin1String("btnResend"));

    if (btn) {
        QMenu *menu = new QMenu(btn);
        QAction *resend = new QAction(i18n("Manual ReSend"), menu);
        connect(resend, &QAction::triggered, this, &GNUSocialApiPostWidget::slotResendPost);
        QAction *repeat = new QAction(i18n("Repeat"), menu);
        repeat->setToolTip(i18n("Repeat post using API"));
        connect(repeat, &QAction::triggered, this, &GNUSocialApiPostWidget::repeatPost);
        menu->addAction(repeat);
        menu->addAction(resend);
        btn->setMenu(menu);
    }
}

GNUSocialApiPostWidget::~GNUSocialApiPostWidget()
{
    delete d;
}

void GNUSocialApiPostWidget::slotReplyToAll()
{
    QStringList nicks;
    nicks.append(currentPost()->author.userName);

    QString txt = QStringLiteral("@%1 ").arg(currentPost()->author.userName);

    int pos = 0;
    while ((pos = mGNUSocialApiUserRegExp.indexIn(currentPost()->content, pos)) != -1) {
        if (mGNUSocialApiUserRegExp.cap(2).toLower() != currentAccount()->username() &&
                mGNUSocialApiUserRegExp.cap(2).toLower() != currentPost()->author.userName &&
                !nicks.contains(mGNUSocialApiUserRegExp.cap(2).toLower())) {
            nicks.append(mGNUSocialApiUserRegExp.cap(2));
            txt += QStringLiteral("@%1 ").arg(mGNUSocialApiUserRegExp.cap(2));
        }
        pos += mGNUSocialApiUserRegExp.matchedLength();
    }

    txt.chop(1);

    Q_EMIT reply(txt, currentPost()->postId, currentPost()->author.userName);
}

QString GNUSocialApiPostWidget::prepareStatus(const QString &text)
{
    QString res = TwitterApiPostWidget::prepareStatus(text);
    res.replace(mStatusNetUserRegExp, QLatin1String("\\1@<a href='user://\\2'>\\2</a>"));
    res.replace(mGNUSocialApiUserRegExp, QLatin1String("\\1@<a href='user://\\2'>\\2</a>"));
    res.replace(mGroupRegExp, QLatin1String("\\1!<a href='group://\\2'>\\2</a>"));
    res.replace(mGNUSocialApiHashRegExp, QLatin1String("\\1#<a href='tag://\\2'>\\2</a>"));

    return res;
}

QString GNUSocialApiPostWidget::generateSign()
{
    return TwitterApiPostWidget::generateSign();
}

void GNUSocialApiPostWidget::checkAnchor(const QUrl &url)
{
    QString scheme = url.scheme();
    QAction *ret;
    if (scheme == QLatin1String("tag")) {
        QString unpcode = QUrl::fromAce(url.host().toUtf8());
        unpcode.remove(QLatin1Char('.'));
        unpcode.remove(QLatin1Char('-'));
        unpcode.remove(QLatin1Char('_'));

        QMenu menu;
        QAction *search = new QAction(QIcon::fromTheme(QLatin1String("system-search")),
                                      i18n("Search for %1", unpcode), &menu);
        QAction *openInBrowser = new QAction(QIcon::fromTheme(QLatin1String("internet-services")),
                                             i18n("Open tag page in browser"), &menu);
        menu.addAction(search);
        menu.addAction(openInBrowser);
        menu.setDefaultAction(search);
        ret = menu.exec(QCursor::pos());
        if (ret == search) {
            d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                    unpcode,
                    GNUSocialApiSearch::ReferenceHashtag);
        } else if (ret == openInBrowser) {
            Choqok::openUrl(QUrl(d->account->homepageUrl().toDisplayString() +
                                 QLatin1String("/tag/") + unpcode));
        }
    } else if (scheme == QLatin1String("group")) {
        QMenu menu;
        QAction *search = new QAction(QIcon::fromTheme(QLatin1String("system-search")),
                                      i18n("Show latest group posts"), &menu);
        QAction *openInBrowser = new QAction(QIcon::fromTheme(QLatin1String("applications-internet")),
                                             i18n("Open group page in browser"), &menu);
        menu.addAction(search);
        menu.addAction(openInBrowser);
        menu.setDefaultAction(search);
        ret = menu.exec(QCursor::pos());
        if (ret == search) {
            d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                    url.host(),
                    GNUSocialApiSearch::ReferenceGroup);
        } else if (ret == openInBrowser) {
            Choqok::openUrl(QUrl(d->account->homepageUrl().toDisplayString() +
                                 QLatin1String("/group/") + url.host()));
        }
    } else if (scheme == QLatin1String("user")) {
        QString username = (url.userName().isEmpty() ? QString() : QStringLiteral("%1@").arg(url.userName())) +
                           url.host();
        QMenu menu;
        QAction *info = new QAction(QIcon::fromTheme(QLatin1String("user-identity")), i18nc("Who is user", "Who is %1",
                                    username), &menu);
        QAction *from = new QAction(QIcon::fromTheme(QLatin1String("edit-find-user")), i18nc("Posts from user", "Posts from %1",
                                    username), &menu);
        QAction *to = new QAction(QIcon::fromTheme(QLatin1String("meeting-attending")), i18nc("Replies to user", "Replies to %1",
                                  username), &menu);
        QAction *openInBrowser = new QAction(QIcon::fromTheme(QLatin1String("applications-internet")),
                                             i18nc("Open profile page in browser",
                                                     "Open profile in browser"), &menu);
        menu.addAction(info);
        if (currentPost()->source != QLatin1String("ostatus")) {
            menu.addAction(from);
            menu.addAction(to);
            from->setData(GNUSocialApiSearch::FromUser);
            to->setData(GNUSocialApiSearch::ToUser);
        }
        menu.addAction(openInBrowser);

        //Subscribe/UnSubscribe/Block
        bool isSubscribe = false;
        QString accountUsername = d->account->username().toLower();
        QString postUsername = username.toLower();
        QAction *subscribe = nullptr, *block = nullptr, *replyTo = nullptr, *dMessage = nullptr;
        if (accountUsername != postUsername) {
            menu.addSeparator();
            QMenu *actionsMenu = menu.addMenu(QIcon::fromTheme(QLatin1String("applications-system")), i18n("Actions"));
            replyTo = new QAction(QIcon::fromTheme(QLatin1String("edit-undo")), i18nc("Write a message to user attention", "Write to %1",
                                  username), actionsMenu);
            actionsMenu->addAction(replyTo);
            if (d->account->friendsList().contains(username)) {
                dMessage = new QAction(QIcon::fromTheme(QLatin1String("mail-message-new")), i18nc("Send direct message to user",
                                       "Send private message to %1",
                                       username), actionsMenu);
                actionsMenu->addAction(dMessage);
                isSubscribe = false;//It's UnSubscribe
                subscribe = new QAction(QIcon::fromTheme(QLatin1String("list-remove-user")),
                                        i18nc("Unsubscribe from user",
                                              "Unsubscribe from %1", username), actionsMenu);
            } else {
                isSubscribe = true;
                subscribe = new QAction(QIcon::fromTheme(QLatin1String("list-add-user")),
                                        i18nc("Subscribe to user",
                                              "Subscribe to %1", username), actionsMenu);
            }
            block = new QAction(QIcon::fromTheme(QLatin1String("dialog-cancel")),
                                i18nc("Block user",
                                      "Block %1", username), actionsMenu);
            if (currentPost()->source != QLatin1String("ostatus")) {
                actionsMenu->addAction(subscribe);
                actionsMenu->addAction(block);
            }
        }
        ret = menu.exec(QCursor::pos());
        if (ret == nullptr) {
            return;
        }
        if (ret == info) {
            TwitterApiWhoisWidget *wd = new TwitterApiWhoisWidget(d->account, username, *currentPost(), this);
            wd->show(QCursor::pos());
            return;
        } else if (ret == subscribe) {
            if (isSubscribe) {
                d->mBlog->createFriendship(d->account, username);
            } else {
                d->mBlog->destroyFriendship(d->account, username);
            }
            return;
        } else if (ret == block) {
            d->mBlog->blockUser(d->account, username);
            return;
        } else if (ret == openInBrowser) {
            Choqok::openUrl(currentAccount()->microblog()->profileUrl(currentAccount(), username));
            return;
        } else if (ret == replyTo) {
            Q_EMIT reply(QStringLiteral("@%1").arg(username), QString(), username);
            return;
        } else if (ret == dMessage) {
            d->mBlog->showDirectMessageDialog(d->account, username);
            return;
        }
        int type = ret->data().toInt();
        d->mBlog->searchBackend()->requestSearchResults(currentAccount(),
                url.host(),
                type);
    } else if (scheme == QLatin1String("conversation")) {
        GNUSocialApiConversationTimelineWidget *tm = new GNUSocialApiConversationTimelineWidget(currentAccount(),
                url.host());
        connect(tm, &GNUSocialApiConversationTimelineWidget::forwardReply, this, &GNUSocialApiPostWidget::reply);
        connect(tm, &GNUSocialApiConversationTimelineWidget::forwardResendPost, this, &GNUSocialApiPostWidget::resendPost);
        tm->show();
    } else {
        TwitterApiPostWidget::checkAnchor(url);
    }
}

void GNUSocialApiPostWidget::slotResendPost()
{
    QString text = generateResendText();

    if (d->account->isChangeExclamationMark()) {
        int index = 0;
        while (true) {
            index = mGroupRegExp.indexIn(text, index);
            if (index != -1) {
                text.replace(index + 1, 1, d->account->changeExclamationMarkToText());
            } else {
                break;
            }
        }
    }

    if ((Choqok::BehaviorSettings::resendWithQuickPost() || currentAccount()->isReadOnly()) &&
            Choqok::UI::Global::quickPostWidget()) {
        Choqok::UI::Global::quickPostWidget()->setText(text);
    } else {
        Q_EMIT resendPost(text);
    }
}
