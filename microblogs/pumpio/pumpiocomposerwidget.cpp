/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>
    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pumpiocomposerwidget.h"

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

#include "pumpiodebug.h"
#include "pumpiomicroblog.h"
#include "pumpiopost.h"

class PumpIOComposerWidget::Private
{
public:
    QString mediumToAttach;
    QPushButton *btnAttach;
    QPointer<QLabel> mediumName;
    QPointer<QPushButton> btnCancel;
    QGridLayout *editorLayout;
    QString replyToObjectType;
};

PumpIOComposerWidget::PumpIOComposerWidget(Choqok::Account *account, QWidget *parent)
    : ComposerWidget(account, parent)
    , d(new Private)
{
    d->editorLayout = qobject_cast<QGridLayout *>(editorContainer()->layout());
    d->btnAttach = new QPushButton(editorContainer());
    d->btnAttach->setIcon(QIcon::fromTheme(QLatin1String("mail-attachment")));
    d->btnAttach->setToolTip(i18n("Attach a file"));
    d->btnAttach->setMaximumWidth(d->btnAttach->height());
    connect(d->btnAttach, &QPushButton::clicked, this, &PumpIOComposerWidget::attachMedia);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->btnAttach);
    vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
    d->editorLayout->addItem(vLayout, 0, 1);
}

PumpIOComposerWidget::~PumpIOComposerWidget()
{
    delete d;
}

void PumpIOComposerWidget::submitPost(const QString &text)
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
            &PumpIOComposerWidget::slotPostSubmited);
    connect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost, this,
            &PumpIOComposerWidget::slotErrorPost);
    btnAbort = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("Abort"), this);
    layout()->addWidget(btnAbort);
    connect(btnAbort, &QPushButton::clicked, this, &PumpIOComposerWidget::abort);

    PumpIOMicroBlog *mBlog = qobject_cast<PumpIOMicroBlog * >(currentAccount()->microblog());
    if (d->mediumToAttach.isEmpty()) {
        if (replyToId.isEmpty()) {
            currentAccount()->microblog()->createPost(currentAccount(), postToSubmit());
        } else {
            // WTF? It seems we cannot cast postToSubmit to PumpIOPost and then I'm copying its attributes
            PumpIOPost *pumpPost = new PumpIOPost();
            pumpPost->content = postToSubmit()->content;
            pumpPost->replyToPostId = postToSubmit()->replyToPostId;
            pumpPost->replyToObjectType = d->replyToObjectType;
            setPostToSubmit(pumpPost);

            mBlog->createReply(currentAccount(), pumpPost);
        }
    } else {
        mBlog->createPostWithMedia(currentAccount(), postToSubmit(), d->mediumToAttach);
    }
}

void PumpIOComposerWidget::slotPostSubmited(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (currentAccount() == theAccount && post == postToSubmit()) {
        qCDebug(CHOQOK) << "Accepted";
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::postCreated,
                   this, &PumpIOComposerWidget::slotPostSubmited);
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost,
                   this, &PumpIOComposerWidget::slotErrorPost);
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

void PumpIOComposerWidget::attachMedia()
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
        connect(d->btnCancel, &QPushButton::clicked, this, &PumpIOComposerWidget::cancelAttach);

        d->editorLayout->addWidget(d->mediumName, 1, 0);
        d->editorLayout->addWidget(d->btnCancel, 1, 1);
    }
    d->mediumName->setText(i18n("Attaching <b>%1</b>", fileName));
    editor()->setFocus();
}

void PumpIOComposerWidget::cancelAttach()
{
    qCDebug(CHOQOK);
    delete d->mediumName;
    d->mediumName = nullptr;
    delete d->btnCancel;
    d->btnCancel = nullptr;
    d->mediumToAttach.clear();
}

void PumpIOComposerWidget::slotSetReply(const QString replyToId, const QString replyToUsername, const QString replyToObjectType)
{
    qCDebug(CHOQOK);
    this->replyToId = replyToId;
    this->replyToUsername = replyToUsername;
    d->replyToObjectType = replyToObjectType;

    if (!replyToUsername.isEmpty()) {
        replyToUsernameLabel()->setText(i18n("Replying to <b>%1</b>", replyToUsername));
        btnCancelReply()->show();
        replyToUsernameLabel()->show();
    }
    editor()->setFocus();
}

#include "moc_pumpiocomposerwidget.cpp"
