/*
    This file is part of Choqok, the KDE micro-blogging client

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

#include "twittercomposerwidget.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QStringListModel>
#include <QVBoxLayout>

#include <KLocalizedString>

#include "account.h"
#include "choqoktextedit.h"
#include "notifymanager.h"
#include "shortenmanager.h"

#include "twitterapiaccount.h"

#include "twitterdebug.h"
#include "twittermicroblog.h"
#include "twittertextedit.h"

class TwitterComposerWidget::Private
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

TwitterComposerWidget::TwitterComposerWidget(Choqok::Account *account, QWidget *parent)
    : TwitterApiComposerWidget(account, parent), d(new Private)
{
    TwitterTextEdit *edit = new TwitterTextEdit(account, this);
    QStringListModel *model = new QStringListModel(qobject_cast<TwitterApiAccount *>(account)->friendsList(), this);
    QCompleter *completer = new QCompleter(model, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    edit->setCompleter(completer);
    setEditor(edit);

    d->editorLayout = qobject_cast<QGridLayout *>(editorContainer()->layout());
    d->btnAttach = new QPushButton(editorContainer());
    d->btnAttach->setIcon(QIcon::fromTheme(QLatin1String("mail-attachment")));
    d->btnAttach->setToolTip(i18n("Attach a file"));
    d->btnAttach->setMaximumWidth(d->btnAttach->height());
    connect(d->btnAttach, &QPushButton::clicked, this, &TwitterComposerWidget::selectMediumToAttach);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->btnAttach);
    vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
    d->editorLayout->addItem(vLayout, 0, 1, 1, 1);
}

TwitterComposerWidget::~TwitterComposerWidget()
{
    delete d;
}

void TwitterComposerWidget::submitPost(const QString &txt)
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
        connect(currentAccount()->microblog(), &Choqok::MicroBlog::postCreated, this,
                &TwitterComposerWidget::slotPostMediaSubmitted);
        connect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost, this,
                &TwitterComposerWidget::slotErrorPost);
        btnAbort = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("Abort"), this);
        layout()->addWidget(btnAbort);
        connect(btnAbort, &QPushButton::clicked, this, &TwitterComposerWidget::abort);
        TwitterMicroBlog *mBlog = qobject_cast<TwitterMicroBlog *>(currentAccount()->microblog());
        mBlog->createPostWithAttachment(currentAccount(), postToSubmit(), d->mediumToAttach);
    }
}

void TwitterComposerWidget::slotPostMediaSubmitted(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (currentAccount() == theAccount && post == postToSubmit()) {
        qCDebug(CHOQOK) << "Accepted";
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::postCreated,
                   this, &TwitterComposerWidget::slotPostMediaSubmitted);
        disconnect(currentAccount()->microblog(), &Choqok::MicroBlog::errorPost,
                   this, &TwitterComposerWidget::slotErrorPost);
        if (btnAbort) {
            btnAbort->deleteLater();
        }
        Choqok::NotifyManager::success(i18n("New post for account %1 submitted successfully", theAccount->alias()));
        editor()->clear();
        replyToId.clear();
        editorContainer()->setEnabled(true);
        setPostToSubmit(nullptr);
        cancelAttachMedium();
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void TwitterComposerWidget::selectMediumToAttach()
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
        connect(d->btnCancel, &QPushButton::clicked, this, &TwitterComposerWidget::cancelAttachMedium);

        d->editorLayout->addWidget(d->mediumName, 1, 0);
        d->editorLayout->addWidget(d->btnCancel, 1, 1);
    }
    d->mediumName->setText(i18n("Attaching <b>%1</b>", fileName));
    editor()->setFocus();
}

void TwitterComposerWidget::cancelAttachMedium()
{
    qCDebug(CHOQOK);
    delete d->mediumName;
    d->mediumName = nullptr;
    delete d->btnCancel;
    d->btnCancel = nullptr;
    d->mediumToAttach.clear();
}

