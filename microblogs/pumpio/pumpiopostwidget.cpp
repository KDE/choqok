/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013-2014 Andrea Scarpino <scarpino@kde.org>
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

#include "pumpiopostwidget.h"

#include <QAction>
#include <QMenu>
#include <QPushButton>

#include <KLocalizedString>

#include "mediamanager.h"
#include "textbrowser.h"

#include "pumpioaccount.h"
#include "pumpiodebug.h"
#include "pumpiomicroblog.h"
#include "pumpiopost.h"
#include "pumpioshowthread.h"

const QIcon PumpIOPostWidget::unFavIcon(Choqok::MediaManager::convertToGrayScale(QIcon::fromTheme(QLatin1String("rating")).pixmap(16)));

class PumpIOPostWidget::Private
{
public:
    QPushButton *btnFavorite;
    QPushButton *btnReply;
};

PumpIOPostWidget::PumpIOPostWidget(Choqok::Account *account, Choqok::Post *post,
                                   QWidget *parent):
    PostWidget(account, post, parent), d(new Private)
{
    mainWidget()->document()->addResource(QTextDocument::ImageResource,
                                          QUrl(QLatin1String("icon://thread")),
                                          QIcon::fromTheme(QLatin1String("go-top")).pixmap(10));
}

PumpIOPostWidget::~PumpIOPostWidget()
{
    delete d;
}

void PumpIOPostWidget::checkAnchor(const QUrl &url)
{
    if (url.scheme() == QLatin1String("thread")) {
        PumpIOShowThread *thread = new PumpIOShowThread(currentAccount(), currentPost());
        connect(thread, SIGNAL(forwardReply(QString,QString,QString)),
                this, SIGNAL(reply(QString,QString,QString)));
        thread->resize(this->width(), thread->height() * 3);
        thread->show();
    } else {
        Choqok::UI::PostWidget::checkAnchor(url);
    }
}

QString PumpIOPostWidget::generateSign()
{
    QString ss;

    PumpIOPost *post = dynamic_cast<PumpIOPost * >(currentPost());
    PumpIOAccount *account = qobject_cast<PumpIOAccount * >(currentAccount());
    PumpIOMicroBlog *microblog = qobject_cast<PumpIOMicroBlog * >(account->microblog());
    if (post) {
        if (post->author.userName != account->username()) {
            ss += QLatin1String("<b><a href=\"") + microblog->profileUrl(account, post->author.homePageUrl)
                  + QLatin1String("\" title=\"") + post->author.realName + QLatin1String("\">") +
                  post->author.userName + QLatin1String("</a></b> - ");
        }

        ss += QLatin1String("<a href=\"") + microblog->postUrl(account, post->author.userName,
                                                post->postId) + QLatin1String("\" title=\"") +
              post->creationDateTime.toString(Qt::DefaultLocaleLongDate)
              + QLatin1String("\">%1</a>");

        if (!post->source.isEmpty()) {
            ss += QLatin1String(" - ") + post->source;
        }

        const QRegExp followers(QLatin1String("/api/user/\\w+/followers"));
        if (!post->to.isEmpty()) {
            ss += QLatin1String(" - ");
            ss += i18n("To:") + QLatin1Char(' ');

            for (const QString &id: post->to) {
                if (id == PumpIOMicroBlog::PublicCollection) {
                    ss += i18n("Public") + QLatin1String(", ");
                } else if (followers.indexIn(id) != -1) {
                    ss += QLatin1String("<a href=\"") + QString(id).remove(QLatin1String("/api/user")) + QLatin1String("\">")
                          + i18n("Followers") + QLatin1String("</a>, ");
                } else if (id == QLatin1String("acct:") + account->webfingerID()) {
                    ss += i18n("You") + QLatin1String(", ");
                } else {
                    ss += QLatin1String("<a href=\"") + microblog->profileUrl(account, id)
                          + QLatin1String("\">") + PumpIOMicroBlog::userNameFromAcct(id) + QLatin1String("</a>, ");
                }
            }

            if (ss.endsWith(QLatin1String(", "))) {
                ss.chop(2);
            }
        }

        if (!post->cc.isEmpty()) {
            ss += QLatin1String(" - ");
            ss += i18n("CC:") + QLatin1Char(' ');

            for (const QString &id: post->cc) {
                if (id == PumpIOMicroBlog::PublicCollection) {
                    ss += i18n("Public") + QLatin1String(", ");
                } else if (followers.indexIn(id) != -1) {
                    ss += QLatin1String("<a href=\"") + QString(id).remove(QLatin1String("/api/user")) + QLatin1String("\">")
                          + i18n("Followers") + QLatin1String("</a>, ");
                } else if (id == QLatin1String("acct:") + account->webfingerID()) {
                    ss += i18n("You") + QLatin1String(", ");
                } else {
                    ss += QLatin1String("<a href=\"") + microblog->profileUrl(account, id)
                          + QLatin1String("\">") + PumpIOMicroBlog::userNameFromAcct(id) + QLatin1String("</a>, ");
                }
            }

            if (ss.endsWith(QLatin1String(", "))) {
                ss.chop(2);
            }
        }

        if (!post->shares.isEmpty()) {
            ss += QLatin1String(" - ");
            ss += i18n("Shared by:") + QLatin1Char(' ');

            for (const QString &id: post->shares) {
                if (id == QLatin1String("acct:") + account->webfingerID()) {
                    ss += i18n("You") + QLatin1String(", ");
                } else {
                    ss += QLatin1String("<a href=\"") + microblog->profileUrl(account, id)
                          + QLatin1String("\">") + PumpIOMicroBlog::userNameFromAcct(id) + QLatin1String("</a>, ");
                }
            }

            if (ss.endsWith(QLatin1String(", "))) {
                ss.chop(2);
            }
        }

        ss += QLatin1String(" <a href=\"thread://\" title=\"") + i18n("Show conversation") + QLatin1String("\"><img src=\"icon://thread\"/></a>");
    } else {
        qCDebug(CHOQOK) << "post is not a PumpIOPost!";
    }

    return ss;
}

void PumpIOPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    if (isResendAvailable()) {
        buttons().value(QLatin1String("btnResend"))->setToolTip(i18nc("@info:tooltip", "Share"));
    }

    if (isReplyAvailable()) {
        d->btnReply = addButton(QLatin1String("btnReply"), i18nc("@info:tooltip", "Reply"), QLatin1String("edit-undo"));
        QMenu *replyMenu = new QMenu(d->btnReply);

        QAction *replyToAct = new QAction(QIcon::fromTheme(QLatin1String("edit-undo")), i18n("Reply to %1",
                                          currentPost()->author.userName), replyMenu);
        replyMenu->addAction(replyToAct);
        connect(replyToAct, SIGNAL(triggered(bool)), SLOT(slotReplyTo()));
        connect(d->btnReply, SIGNAL(clicked(bool)), SLOT(slotReplyTo()));
    }

    d->btnFavorite = addButton(QLatin1String("btnFavorite"), i18nc("@info:tooltip", "Like"), QLatin1String("rating"));
    d->btnFavorite->setCheckable(true);
    connect(d->btnFavorite, SIGNAL(clicked(bool)), this, SLOT(toggleFavorite()));
    updateFavStat();
}

void PumpIOPostWidget::toggleFavorite()
{
    qCDebug(CHOQOK);
    setReadWithSignal();
    PumpIOMicroBlog *microBlog = qobject_cast<PumpIOMicroBlog *>(currentAccount()->microblog());
    connect(microBlog, SIGNAL(favorite(Choqok::Account*,Choqok::Post*)),
            this, SLOT(slotToggleFavorite(Choqok::Account*,Choqok::Post*)));
    microBlog->toggleFavorite(currentAccount(), currentPost());
}

void PumpIOPostWidget::slotToggleFavorite(Choqok::Account *, Choqok::Post *)
{
    qCDebug(CHOQOK);
    updateFavStat();
}

void PumpIOPostWidget::slotPostError(Choqok::Account *theAccount, Choqok::Post *post,
                                     Choqok::MicroBlog::ErrorType error,
                                     const QString &errorMessage)
{
    Q_UNUSED(error)

    qCDebug(CHOQOK);
    if (theAccount == currentAccount() && post == currentPost()) {
        qCDebug(CHOQOK) << errorMessage;
        disconnect(currentAccount()->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                   this, SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)));
        disconnect(currentAccount()->microblog(),
                   SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,QString,Choqok::MicroBlog::ErrorLevel)),
                   this, SLOT(slotPostError(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)));
    }
}

void PumpIOPostWidget::slotResendPost()
{
    qCDebug(CHOQOK);
    setReadWithSignal();
    PumpIOMicroBlog *microBlog = qobject_cast<PumpIOMicroBlog *>(currentAccount()->microblog());
    microBlog->share(currentAccount(), currentPost());
}

bool PumpIOPostWidget::isReplyAvailable()
{
    return (currentPost()->type != QLatin1String("comment"));
}

bool PumpIOPostWidget::isResendAvailable()
{
    return PostWidget::isResendAvailable() && (currentPost()->type != QLatin1String("comment"));
}

void PumpIOPostWidget::slotReplyTo()
{
    qCDebug(CHOQOK);
    setReadWithSignal();
    PumpIOPost *post = dynamic_cast<PumpIOPost * >(currentPost());
    if (post->type == QLatin1String("comment")) {
        Q_EMIT reply(post->replyToPostId, post->replyToUserName, post->replyToObjectType);
    } else {
        Q_EMIT reply(post->postId, PumpIOMicroBlog::userNameFromAcct(post->author.userId), post->type);
    }
}

void PumpIOPostWidget::updateFavStat()
{
    d->btnFavorite->setChecked(currentPost()->isFavorited);
    if (currentPost()->isFavorited) {
        d->btnFavorite->setIcon(QIcon::fromTheme(QLatin1String("rating")));
    } else {
        d->btnFavorite->setIcon(unFavIcon);
    }
}
