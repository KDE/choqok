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

#include <QMenu>

#include <KAction>
#include <KDebug>
#include <KLocalizedString>
#include <KPushButton>

#include "mediamanager.h"
#include "textbrowser.h"

#include "pumpioaccount.h"
#include "pumpiomicroblog.h"
#include "pumpiopost.h"
#include "pumpioshowthread.h"

const KIcon PumpIOPostWidget::unFavIcon(Choqok::MediaManager::convertToGrayScale(KIcon("rating").pixmap(16)));

class PumpIOPostWidget::Private
{
public:
    KPushButton *btnFavorite;
    KPushButton *btnReply;
};

PumpIOPostWidget::PumpIOPostWidget(Choqok::Account* account, Choqok::Post* post,
                                   QWidget* parent):
    PostWidget(account, post, parent), d(new Private)
{
    mainWidget()->document()->addResource(QTextDocument::ImageResource,
                                          QUrl("icon://thread"),
                                          KIcon("go-top").pixmap(10));
}

PumpIOPostWidget::~PumpIOPostWidget()
{
    delete d;
}

void PumpIOPostWidget::checkAnchor(const QUrl& url)
{
    if (url.scheme() == "thread") {
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

    PumpIOPost *post = dynamic_cast<PumpIOPost* >(currentPost());
    PumpIOAccount *account = qobject_cast<PumpIOAccount* >(currentAccount());
    PumpIOMicroBlog *microblog = qobject_cast<PumpIOMicroBlog* >(account->microblog());
    if (post) {
        if (post->author.userName != account->username()) {
            ss += "<b><a href='"+ microblog->profileUrl(account, post->author.homePageUrl)
                  + "' title=\"" + post->author.realName + "\">" +
                  post->author.userName + "</a></b> - ";
        }

        ss += "<a href=\"" + microblog->postUrl(account, post->author.userName,
              post->postId) + "\" title=\"" +
              post->creationDateTime.toString(Qt::DefaultLocaleLongDate)
              + "\">%1</a>";

        if (!post->source.isEmpty()) {
            ss += " - " + post->source;
        }

        const QRegExp followers("/api/user/\\w+/followers");
        if (!post->to.isEmpty()) {
            ss += " - ";
            ss += i18n("To:") + ' ';

            Q_FOREACH (const QString& id, post->to) {
                if (id == "http://activityschema.org/collection/public") {
                    ss += i18n("Public") + ' ';
                } else if (followers.indexIn(id) != -1) {
                    ss += "<a href=\"" + QString(id).remove("/api/user") + "\">"
                          + i18n("Followers") + "</a> ";
                } else if (id == "acct:" + account->webfingerID()) {
                    ss += i18n("You") + ' ';
                } else {
                    ss += "<a href=\"" + microblog->profileUrl(account, id)
                          + "\">" + PumpIOMicroBlog::userNameFromAcct(id) + "</a> ";
                }
            }
            ss = ss.trimmed();
        }

        if (!post->cc.isEmpty()) {
            ss += " - ";
            ss += i18n("CC:") + ' ';

            Q_FOREACH (const QString& id, post->cc) {
                if (id == "http://activityschema.org/collection/public") {
                    ss += i18n("Public") + ' ';
                } else if (followers.indexIn(id) != -1) {
                    ss += "<a href=\"" + QString(id).remove("/api/user") + "\">"
                          + i18n("Followers") + "</a> ";
                } else if (id == "acct:" + account->webfingerID()) {
                    ss += i18n("You") + ' ';
                } else {
                    ss += "<a href=\"" + microblog->profileUrl(account, id)
                          + "\">" + PumpIOMicroBlog::userNameFromAcct(id) + "</a> ";
                }
            }
            ss = ss.trimmed();
        }

        if (!post->shares.isEmpty()) {
            ss += " - ";
            ss += i18n("Shared by:") + ' ';

            Q_FOREACH (const QString& id, post->shares) {
                if (id == "acct:" + account->webfingerID()) {
                    ss += i18n("You") + ' ';
                } else {
                    ss += "<a href=\"" + microblog->profileUrl(account, id)
                          + "\">" + PumpIOMicroBlog::userNameFromAcct(id) + "</a> ";
                }
            }
            ss = ss.trimmed();
        }

        ss += " - ";
        ss += i18n("View replies");
        ss += " <a href=\"thread://\"><img src=\"icon://thread\"/></a>";
    } else {
        kDebug() << "post is not a PumpIOPost!";
    }

    return ss;
}

void PumpIOPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    if (isResendAvailable()) {
        buttons().value("btnResend")->setToolTip(i18nc("@info:tooltip", "Share"));
    }

    if (isReplyAvailable()) {
        d->btnReply = addButton("btnReply", i18nc("@info:tooltip", "Reply"), "edit-undo");
        QMenu *replyMenu = new QMenu(d->btnReply);

        KAction *replyToAct = new KAction(KIcon("edit-undo"), i18n("Reply to %1",
                                      currentPost()->author.userName), replyMenu);
        replyMenu->addAction(replyToAct);
        connect(replyToAct, SIGNAL(triggered(bool)), SLOT(slotReplyTo()));
        connect(d->btnReply, SIGNAL(clicked(bool)), SLOT(slotReplyTo()));
    }

    d->btnFavorite = addButton("btnFavorite", i18nc("@info:tooltip", "Like"), "rating");
    d->btnFavorite->setCheckable(true);
    connect(d->btnFavorite, SIGNAL(clicked(bool)), this, SLOT(toggleFavorite()));
    updateFavStat();
}

void PumpIOPostWidget::toggleFavorite()
{
    kDebug();
    setReadWithSignal();
    PumpIOMicroBlog* microBlog = qobject_cast<PumpIOMicroBlog*>(currentAccount()->microblog());
    connect(microBlog, SIGNAL(favorite(Choqok::Account*, Choqok::Post*)),
            this, SLOT(slotToggleFavorite(Choqok::Account*, Choqok::Post*)));
    microBlog->toggleFavorite(currentAccount(), currentPost());
}

void PumpIOPostWidget::slotToggleFavorite(Choqok::Account*, Choqok::Post*)
{
    kDebug();
    updateFavStat();
}

void PumpIOPostWidget::slotPostError(Choqok::Account* theAccount, Choqok::Post* post,
                                     Choqok::MicroBlog::ErrorType error,
                                     const QString& errorMessage)
{
    Q_UNUSED(error)

    kDebug();
    if (theAccount == currentAccount() && post == currentPost()) {
        kDebug() << errorMessage;
        disconnect(currentAccount()->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                  this, SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)) );
        disconnect(currentAccount()->microblog(),
                   SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,QString,Choqok::MicroBlog::ErrorLevel)),
                   this, SLOT(slotPostError(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)));
    }
}

void PumpIOPostWidget::slotResendPost()
{
    kDebug();
    setReadWithSignal();
    PumpIOMicroBlog* microBlog = qobject_cast<PumpIOMicroBlog*>(currentAccount()->microblog());
    microBlog->share(currentAccount(), currentPost());
}

bool PumpIOPostWidget::isReplyAvailable()
{
    return (currentPost()->type != "comment");
}

bool PumpIOPostWidget::isResendAvailable()
{
    return (currentPost()->type != "comment");
}

void PumpIOPostWidget::slotReplyTo()
{
    kDebug();
    setReadWithSignal();
    PumpIOPost *post = dynamic_cast<PumpIOPost* >(currentPost());
    if (post->type == "comment") {
        emit reply(post->replyToPostId, post->replyToUserName, post->replyToObjectType);
    } else {
        emit reply(post->postId, PumpIOMicroBlog::userNameFromAcct(post->author.userId), post->type);
    }
}

void PumpIOPostWidget::updateFavStat()
{
    d->btnFavorite->setChecked(currentPost()->isFavorited);
    if (currentPost()->isFavorited){
        d->btnFavorite->setIcon(KIcon("rating"));
    } else {
        d->btnFavorite->setIcon(unFavIcon);
    }
}
