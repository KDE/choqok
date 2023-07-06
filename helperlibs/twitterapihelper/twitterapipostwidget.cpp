/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterapipostwidget.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>

#include "choqokappearancesettings.h"
#include "mediamanager.h"
#include "microblog.h"
#include "textbrowser.h"
#include "twitterapiaccount.h"
#include "twitterapidebug.h"
#include "twitterapimicroblog.h"
#include "twitterapishowthread.h"

const QIcon TwitterApiPostWidget::unFavIcon(Choqok::MediaManager::convertToGrayScale(QIcon::fromTheme(QLatin1String("rating")).pixmap(16)));

class TwitterApiPostWidget::Private
{
public:
    Private(Choqok::Account *account)
        : isBasePostShowed(false)
    {
        mBlog = qobject_cast<TwitterApiMicroBlog *>(account->microblog());
    }
    QPushButton *btnFav;
    bool isBasePostShowed;
    TwitterApiMicroBlog *mBlog;
};

TwitterApiPostWidget::TwitterApiPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent)
    : PostWidget(account, post, parent), d(new Private(account))
{
    mainWidget()->document()->addResource(QTextDocument::ImageResource, QUrl(QLatin1String("icon://thread")),
                                          QIcon::fromTheme(QLatin1String("go-top")).pixmap(10));
}

TwitterApiPostWidget::~TwitterApiPostWidget()
{
    delete d;
}

void TwitterApiPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    QPushButton *btnRe = addButton(QLatin1String("btnReply"), i18nc("@info:tooltip", "Reply"), QLatin1String("edit-undo"));
    connect(btnRe, &QPushButton::clicked, this, &TwitterApiPostWidget::slotReply);
    QMenu *menu = new QMenu(btnRe);
    btnRe->setMenu(menu);

    QAction *actRep = new QAction(QIcon::fromTheme(QLatin1String("edit-undo")), i18n("Reply to %1", currentPost()->author.userName), menu);
    menu->addAction(actRep);
    menu->setDefaultAction(actRep);
    connect(actRep, &QAction::triggered, this, &TwitterApiPostWidget::slotReply);

    QAction *actWrite = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Write to %1", currentPost()->author.userName), menu);
    menu->addAction(actWrite);
    connect(actWrite, &QAction::triggered, this, &TwitterApiPostWidget::slotWriteTo);

    if (!currentPost()->isPrivate) {
        QAction *actReplytoAll = new QAction(i18n("Reply to all"), menu);
        menu->addAction(actReplytoAll);
        connect(actReplytoAll, &QAction::triggered, this, &TwitterApiPostWidget::slotReplyToAll);

        d->btnFav = addButton(QLatin1String("btnFavorite"), i18nc("@info:tooltip", "Favorite"), QLatin1String("rating"));
        d->btnFav->setCheckable(true);
        connect(d->btnFav, &QPushButton::clicked, this, &TwitterApiPostWidget::setFavorite);
        updateFavStat();
    }
}

QString TwitterApiPostWidget::generateSign()
{
    QString sign = QStringLiteral("<b>%1 - </b>").arg(getUsernameHyperlink(currentPost()->author));

    //<img src=\"icon://web\" />
    if (currentPost()->isPrivate) {
        sign += QLatin1String("%1");

        if (currentPost()->replyToUser.userName.compare(currentAccount()->username(), Qt::CaseInsensitive) == 0) {
            sign.prepend(QLatin1String("From "));
        } else {
            sign.prepend(QLatin1String("To "));
        }
    } else {
        QDateTime time;
        if (currentPost()->repeatedDateTime.isNull()) {
            time = currentPost()->creationDateTime;
        } else {
            time = currentPost()->repeatedDateTime;
        }

        sign += QStringLiteral("<a href=\"%1\" title=\"%2\">%3</a>").arg(currentPost()->link.toDisplayString())
                .arg(time.toString(Qt::DefaultLocaleLongDate)).arg(QStringLiteral("%1"));
    }

    if (!currentPost()->source.isEmpty()) {
        sign += QLatin1String(" - ");
        if (currentPost()->source == QLatin1String("ostatus") && !currentPost()->author.homePageUrl.isEmpty()) {
            sign += i18n("<a href='%1' title='Sent from %2 via OStatus'>%2</a>",
                         currentPost()->author.homePageUrl.toDisplayString(),
                         currentPost()->author.homePageUrl.host());
        } else {
            sign += currentPost()->source;
        }
    }

    if (!currentPost()->isPrivate) {
        if (!currentPost()->replyToPostId.isEmpty()) {
            QUrl link = currentAccount()->microblog()->postUrl(currentAccount(), currentPost()->replyToUser.userName,
                           currentPost()->replyToPostId);
            QString showConMsg = i18n("Show Conversation");
            QString threadlink;
            if (currentPost()->conversationId.isEmpty()) {
                threadlink = QLatin1String("thread://") + currentPost()->postId;
            } else {
                threadlink = QLatin1String("conversation://") + currentPost()->conversationId;
            }
            sign += QLatin1String(" - ") +
                    i18n("<a href='replyto://%1'>in reply to</a> @<a href='user://%4'>%4</a>&nbsp;<a href=\"%2\" title=\"%2\">%3</a>",
                         currentPost()->replyToPostId, link.toDisplayString(), webIconText, currentPost()->replyToUser.userName) + QLatin1Char(' ');
            sign += QLatin1String("<a title=\"") + showConMsg + QLatin1String("\" href=\"") + threadlink + QLatin1String("\"><img src=\"icon://thread\" /></a>");
        }

        //ReTweet detection
        if (!currentPost()->repeatedFromUser.userName.isEmpty()) {
            const QString retweet = QLatin1String("<br/>") +
                    d->mBlog->generateRepeatedByUserTooltip(QStringLiteral("<a href='user://%1'>%2</a>")
                                                            .arg(currentPost()->repeatedFromUser.userName)
                                                            .arg(currentPost()->repeatedFromUser.userName));
            sign.append(retweet);
        }
    }

    sign.prepend(QLatin1String("<p dir='ltr'>"));
    sign.append(QLatin1String("</p>"));

    return sign;
}

QString TwitterApiPostWidget::getUsernameHyperlink(const Choqok::User &user) const
{
    return QStringLiteral("<a href=\"user://%1\" title=\"%2\">%3</a>")
            .arg(user.userName)
            .arg(user.description.isEmpty() ? user.realName : user.description.toHtmlEscaped())
            .arg(user.userName);
}

void TwitterApiPostWidget::slotReply()
{
    setReadWithSignal();
    if (currentPost()->isPrivate) {
        TwitterApiAccount *account = qobject_cast<TwitterApiAccount *>(currentAccount());
        d->mBlog->showDirectMessageDialog(account, currentPost()->author.userName);
    } else {
        QString replyto = QStringLiteral("@%1").arg(currentPost()->author.userName);
        QString postId = currentPost()->postId;
        QString username = currentPost()->author.userName;
        if (!currentPost()->repeatedFromUser.userName.isEmpty()) {
            replyto.prepend(QStringLiteral("@%1 ").arg(currentPost()->repeatedFromUser.userName));
            postId = currentPost()->repeatedPostId;
        }
        Q_EMIT reply(replyto, postId,  username);
    }
}

void TwitterApiPostWidget::slotWriteTo()
{
    Q_EMIT reply(QStringLiteral("@%1").arg(currentPost()->author.userName), QString(), currentPost()->author.userName);
}

void TwitterApiPostWidget::slotReplyToAll()
{
    QString txt = QStringLiteral("@%1").arg(currentPost()->author.userName);
    Q_EMIT reply(txt, currentPost()->postId, currentPost()->author.userName);
}

void TwitterApiPostWidget::setFavorite()
{
    setReadWithSignal();
    TwitterApiMicroBlog *mic = d->mBlog;
    if (currentPost()->isFavorited) {
        connect(mic, &TwitterApiMicroBlog::favoriteRemoved, this, &TwitterApiPostWidget::slotSetFavorite);
        mic->removeFavorite(currentAccount(), currentPost()->postId);
    } else {
        connect(mic, &TwitterApiMicroBlog::favoriteCreated, this, &TwitterApiPostWidget::slotSetFavorite);
        mic->createFavorite(currentAccount(), currentPost()->postId);
    }
}

void TwitterApiPostWidget::slotSetFavorite(Choqok::Account *theAccount, const QString &postId)
{
    if (currentAccount() == theAccount && postId == currentPost()->postId) {
        qCDebug(CHOQOK) << postId;
        currentPost()->isFavorited = !currentPost()->isFavorited;
        updateFavStat();
        disconnect(d->mBlog, &TwitterApiMicroBlog::favoriteRemoved, this, &TwitterApiPostWidget::slotSetFavorite);
        disconnect(d->mBlog, &TwitterApiMicroBlog::favoriteCreated, this, &TwitterApiPostWidget::slotSetFavorite);
    }
}

void TwitterApiPostWidget::updateFavStat()
{
    if (currentPost()->isFavorited) {
        d->btnFav->setChecked(true);
        d->btnFav->setIcon(QIcon::fromTheme(QLatin1String("rating")));
    } else {
        d->btnFav->setChecked(false);
        d->btnFav->setIcon(unFavIcon);
    }
}

void TwitterApiPostWidget::checkAnchor(const QUrl &url)
{
    QString scheme = url.scheme();
    if (scheme == QLatin1String("replyto")) {
        if (d->isBasePostShowed) {
            setContent(prepareStatus(currentPost()->content).replace(QLatin1String("<a href"), QLatin1String("<a style=\"text-decoration:none\" href"), Qt::CaseInsensitive));
            updateUi();
            d->isBasePostShowed = false;
            return;
        } else {
            connect(currentAccount()->microblog(), &Choqok::MicroBlog::postFetched,
                    this, &TwitterApiPostWidget::slotBasePostFetched);
            Choqok::Post *ps = new Choqok::Post;
            ps->postId = url.host();
            currentAccount()->microblog()->fetchPost(currentAccount(), ps);
        }
    } else if (scheme == QLatin1String("thread")) {
        TwitterApiShowThread *wd = new TwitterApiShowThread(currentAccount(), currentPost(), nullptr);
        wd->resize(this->width(), wd->height());
        connect(wd, &TwitterApiShowThread::forwardReply, this, &TwitterApiPostWidget::reply);
        connect(wd, &TwitterApiShowThread::forwardResendPost, this, &TwitterApiPostWidget::resendPost);
        wd->show();
    } else {
        Choqok::UI::PostWidget::checkAnchor(url);
    }

}

void TwitterApiPostWidget::slotBasePostFetched(Choqok::Account *theAccount, Choqok::Post *post)
{
    if (theAccount == currentAccount() && post && post->postId == currentPost()->replyToPostId) {
        qCDebug(CHOQOK);
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::postFetched,
                   this, &TwitterApiPostWidget::slotBasePostFetched);
        if (d->isBasePostShowed) {
            return;
        }
        d->isBasePostShowed = true;
        QString color;
        if (Choqok::AppearanceSettings::isCustomUi()) {
            color = Choqok::AppearanceSettings::readForeColor().lighter().name();
        } else {
            color = this->palette().dark().color().name();
        }
        QString baseStatusText = QLatin1String("<p style=\"margin-top:10px; margin-bottom:10px; margin-left:20px;\
        margin-right:20px; text-indent:0px\"><span style=\" color:") + color + QLatin1String(";\">");
        baseStatusText += QLatin1String("<b><a href='user://") + post->author.userName + QLatin1String("'>") +
                          post->author.userName + QLatin1String("</a> :</b> ");

        baseStatusText += prepareStatus(post->content) + QLatin1String("</p>");
        setContent(content().prepend(baseStatusText.replace(QLatin1String("<a href"), QLatin1String("<a style=\"text-decoration:none\" href"), Qt::CaseInsensitive)));
        updateUi();
        if( post->owners < 1 )
            delete post;
    }
}

void TwitterApiPostWidget::repeatPost()
{
    setReadWithSignal();
    QString postId;
    if (currentPost()->repeatedPostId.isEmpty()) {
        postId = currentPost()->postId;
    } else {
        postId = currentPost()->repeatedPostId;
    }
    auto q_answer = KMessageBox::questionYesNo(Choqok::UI::Global::mainWindow(), d->mBlog->repeatQuestion(),
                                               QString(), KStandardGuiItem::yes(), KStandardGuiItem::cancel(),
                                               QLatin1String("dontAskRepeatConfirm"));
    if ( q_answer == KMessageBox::Yes) {
        d->mBlog->repeatPost(currentAccount(), postId);
    }
}

#include "moc_twitterapipostwidget.cpp"
