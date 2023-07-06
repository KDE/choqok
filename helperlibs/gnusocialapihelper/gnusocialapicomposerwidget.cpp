/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gnusocialapicomposerwidget.h"

#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QPushButton>

#include <KLocalizedString>

#include "account.h"
#include "choqoktextedit.h"
#include "microblog.h"
#include "notifymanager.h"
#include "shortenmanager.h"

#include "twitterapitextedit.h"

#include "gnusocialapidebug.h"
#include "gnusocialapimicroblog.h"

class GNUSocialApiComposerWidget::Private
{
public:
    Private()
        : btnAttach(nullptr), mediumName(nullptr), btnCancel(nullptr)
    {}
    QString mediumToAttach;
    QPushButton *btnAttach;
    QPointer<QLabel> mediumName;
    QPointer<QPushButton> btnCancel;
    QGridLayout *editorLayout;
};

GNUSocialApiComposerWidget::GNUSocialApiComposerWidget(Choqok::Account *account, QWidget *parent)
    : TwitterApiComposerWidget(account, parent), d(new Private)
{
    d->editorLayout = qobject_cast<QGridLayout *>(editorContainer()->layout());
    d->btnAttach = new QPushButton(editorContainer());
    d->btnAttach->setIcon(QIcon::fromTheme(QLatin1String("mail-attachment")));
    d->btnAttach->setToolTip(i18n("Attach a file"));
    d->btnAttach->setMaximumWidth(d->btnAttach->height());
    connect(d->btnAttach, &QPushButton::clicked, this, &GNUSocialApiComposerWidget::selectMediumToAttach);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->btnAttach);
    vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
    d->editorLayout->addItem(vLayout, 0, 1, 1, 1);

    connect(account, &Choqok::Account::modified, this, &GNUSocialApiComposerWidget::slotRebuildEditor);
}

GNUSocialApiComposerWidget::~GNUSocialApiComposerWidget()
{
    delete d;
}

void GNUSocialApiComposerWidget::submitPost(const QString &txt)
{
    if (d->mediumToAttach.isEmpty()) {
        Choqok::UI::ComposerWidget::submitPost(txt);
    } else {
        qCDebug(CHOQOK);
        editorContainer()->setEnabled(false);
        QString text = txt;
        if (currentAccount()->postCharLimit() &&
                text.size() > (int)currentAccount()->postCharLimit()) {
            text = Choqok::ShortenManager::self()->parseText(text);
        }
        setPostToSubmit(nullptr);
        setPostToSubmit(new Choqok::Post);
        postToSubmit()->content = text;
        if (!replyToId.isEmpty()) {
            postToSubmit()->replyToPostId = replyToId;
        }
        connect(currentAccount()->microblog(), &Choqok::MicroBlog::postCreated,
                this, &GNUSocialApiComposerWidget::slotPostMediaSubmitted);
        connect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost,
                this, &GNUSocialApiComposerWidget::slotErrorPost);
        btnAbort = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("Abort"), this);
        layout()->addWidget(btnAbort);
        connect(btnAbort, &QPushButton::clicked, this, &Choqok::UI::ComposerWidget::abort);
        GNUSocialApiMicroBlog *mBlog = qobject_cast<GNUSocialApiMicroBlog *>(currentAccount()->microblog());
        mBlog->createPostWithAttachment(currentAccount(), postToSubmit(), d->mediumToAttach);
    }
}

void GNUSocialApiComposerWidget::slotPostMediaSubmitted(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (currentAccount() == theAccount && post == postToSubmit()) {
        qCDebug(CHOQOK) << "Accepted";
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::postCreated,
                   this, &GNUSocialApiComposerWidget::slotPostMediaSubmitted);
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost,
                   this, &GNUSocialApiComposerWidget::slotErrorPost);
        if (btnAbort) {
            btnAbort->deleteLater();
        }
        Choqok::NotifyManager::success(i18n("New post for account %1 submitted successfully",
                                            theAccount->alias()));
        editor()->clear();
        replyToId.clear();
        editorContainer()->setEnabled(true);
        setPostToSubmit(nullptr);
        cancelAttachMedium();
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void GNUSocialApiComposerWidget::selectMediumToAttach()
{
    qCDebug(CHOQOK);
    d->mediumToAttach = QFileDialog::getOpenFileName(this, i18n("Select Media to Upload"),
                                                     QString(), QStringLiteral("Images"));
    if (d->mediumToAttach.isEmpty()) {
        return;
    }
    QString fileName = QUrl(d->mediumToAttach).fileName();
    if (!d->mediumName) {
        qCDebug(CHOQOK) << fileName;
        d->mediumName = new QLabel(editorContainer());
        d->btnCancel = new QPushButton(editorContainer());
        d->btnCancel->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
        d->btnCancel->setToolTip(i18n("Discard Attachment"));
        d->btnCancel->setMaximumWidth(d->btnCancel->height());
        connect(d->btnCancel, &QPushButton::clicked, this, &GNUSocialApiComposerWidget::cancelAttachMedium);

        d->editorLayout->addWidget(d->mediumName, 1, 0);
        d->editorLayout->addWidget(d->btnCancel, 1, 1);
    }
    d->mediumName->setText(i18n("Attaching <b>%1</b>", fileName));
    editor()->setFocus();
}

void GNUSocialApiComposerWidget::cancelAttachMedium()
{
    qCDebug(CHOQOK);
    delete d->mediumName;
    d->mediumName = nullptr;
    delete d->btnCancel;
    d->btnCancel = nullptr;
    d->mediumToAttach.clear();
}

void GNUSocialApiComposerWidget::slotRebuildEditor(Choqok::Account *theAccount)
{
    setEditor(new TwitterApiTextEdit(theAccount, this));
}

#include "moc_gnusocialapicomposerwidget.cpp"
