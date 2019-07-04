/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017 Andrea Scarpino <scarpino@kde.org>

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

#include "mastodoncomposerwidget.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <KLocalizedString>

#include "account.h"
#include "choqoktextedit.h"
#include "shortenmanager.h"

#include "mastodondebug.h"
#include "mastodonmicroblog.h"
#include "mastodonpost.h"

class MastodonComposerWidget::Private
{
public:
    QString mediumToAttach;
    QPushButton *btnAttach;
    QPointer<QLabel> mediumName;
    QPointer<QPushButton> btnCancel;
    QGridLayout *editorLayout;
};

MastodonComposerWidget::MastodonComposerWidget(Choqok::Account *account, QWidget *parent)
    : ComposerWidget(account, parent)
    , d(new Private)
{
    d->editorLayout = qobject_cast<QGridLayout *>(editorContainer()->layout());
    d->btnAttach = new QPushButton(editorContainer());
    d->btnAttach->setIcon(QIcon::fromTheme(QLatin1String("mail-attachment")));
    d->btnAttach->setToolTip(i18n("Attach a file"));
    d->btnAttach->setMaximumWidth(d->btnAttach->height());
    connect(d->btnAttach, &QPushButton::clicked, this, &MastodonComposerWidget::attachMedia);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->btnAttach);
    vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
    d->editorLayout->addItem(vLayout, 0, 1);
}

MastodonComposerWidget::~MastodonComposerWidget()
{
    delete d;
}

void MastodonComposerWidget::submitPost(const QString &text)
{
    qCDebug(CHOQOK);
    editorContainer()->setEnabled(false);
    QString txt = text;
    if (currentAccount()->postCharLimit() &&
            txt.size() > (int) currentAccount()->postCharLimit()) {
        txt = Choqok::ShortenManager::self()->parseText(txt);
    }
    setPostToSubmit(nullptr);
    setPostToSubmit(new Choqok::Post);
    postToSubmit()->content = txt;
    if (!replyToId.isEmpty()) {
        postToSubmit()->replyToPostId = replyToId;
    }
    connect(currentAccount()->microblog(), &Choqok::MicroBlog::postCreated, this,
            &MastodonComposerWidget::slotPostSubmited);
    connect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost, this,
            &MastodonComposerWidget::slotErrorPost);
    btnAbort = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("Abort"), this);
    layout()->addWidget(btnAbort);
    connect(btnAbort, &QPushButton::clicked, this, &MastodonComposerWidget::abort);

    MastodonMicroBlog *mBlog = qobject_cast<MastodonMicroBlog * >(currentAccount()->microblog());
    if (d->mediumToAttach.isEmpty()) {
        if (replyToId.isEmpty()) {
            currentAccount()->microblog()->createPost(currentAccount(), postToSubmit());
        } else {
            // WTF? It seems we cannot cast postToSubmit to MastodonPost and then I'm copying its attributes
            MastodonPost *pumpPost = new MastodonPost();
            pumpPost->content = postToSubmit()->content;
            pumpPost->replyToPostId = postToSubmit()->replyToPostId;
            setPostToSubmit(pumpPost);

            mBlog->createReply(currentAccount(), pumpPost);
        }
    } else {
        // TODO
        // mBlog->createPostWithMedia(currentAccount(), postToSubmit(), d->mediumToAttach);
    }
}

void MastodonComposerWidget::slotPostSubmited(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (currentAccount() == theAccount && post == postToSubmit()) {
        qCDebug(CHOQOK) << "Accepted";
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::postCreated,
                   this, &MastodonComposerWidget::slotPostSubmited);
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost,
                   this, &MastodonComposerWidget::slotErrorPost);
        if (btnAbort) {
            btnAbort->deleteLater();
        }
        editor()->clear();
        editorCleared();
        editorContainer()->setEnabled(true);
        setPostToSubmit(nullptr);
        cancelAttach();
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void MastodonComposerWidget::attachMedia()
{
    qCDebug(CHOQOK);
    d->mediumToAttach = QFileDialog::getOpenFileName(this, i18n("Select Media to Upload"),
                                                     QString(), QStringLiteral("Images"));
    if (d->mediumToAttach.isEmpty()) {
        qCDebug(CHOQOK) << "No file selected";
        return;
    }
    const QString fileName = QUrl(d->mediumToAttach).fileName();
    if (!d->mediumName) {
        d->mediumName = new QLabel(editorContainer());
        d->btnCancel = new QPushButton(editorContainer());
        d->btnCancel->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
        d->btnCancel->setToolTip(i18n("Discard Attachment"));
        d->btnCancel->setMaximumWidth(d->btnCancel->height());
        connect(d->btnCancel, &QPushButton::clicked, this, &MastodonComposerWidget::cancelAttach);

        d->editorLayout->addWidget(d->mediumName, 1, 0);
        d->editorLayout->addWidget(d->btnCancel, 1, 1);
    }
    d->mediumName->setText(i18n("Attaching <b>%1</b>", fileName));
    editor()->setFocus();
}

void MastodonComposerWidget::cancelAttach()
{
    qCDebug(CHOQOK);
    delete d->mediumName;
    d->mediumName = 0;
    delete d->btnCancel;
    d->btnCancel = 0;
    d->mediumToAttach.clear();
}
