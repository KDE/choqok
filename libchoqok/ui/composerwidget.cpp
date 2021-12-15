/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "composerwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>

#include "account.h"
#include "choqoktextedit.h"
#include "libchoqokdebug.h"
#include "microblog.h"
#include "notifymanager.h"
#include "shortenmanager.h"

namespace Choqok
{
namespace UI
{

class ComposerWidget::Private
{
public:
    Private(Account *account)
        : editor(nullptr), currentAccount(account), postToSubmit(nullptr)
    {}
    QPointer<TextEdit> editor;
    Account *currentAccount;
    Choqok::Post *postToSubmit;
    QWidget *editorContainer;
    QPointer<QLabel> replyToUsernameLabel;
    QPointer<QPushButton> btnCancelReply;
};

ComposerWidget::ComposerWidget(Choqok::Account *account, QWidget *parent /*= 0*/)
    : QWidget(parent), btnAbort(nullptr), d(new Private(account))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    d->editorContainer = new QWidget(this);
    QGridLayout *internalLayout = new QGridLayout;
    internalLayout->setContentsMargins(0, 0, 0, 0);
    d->editorContainer->setLayout(internalLayout);
    layout->addWidget(editorContainer());
    setEditor(new TextEdit(account->postCharLimit(), this));

    d->replyToUsernameLabel = new QLabel(editorContainer());
    d->btnCancelReply = new QPushButton(editorContainer());
    d->btnCancelReply->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));
    d->btnCancelReply->setToolTip(i18n("Discard Reply"));
    d->btnCancelReply->setMaximumWidth(d->btnCancelReply->height());
    connect(d->btnCancelReply, &QPushButton::clicked, this, &ComposerWidget::editorCleared);
    internalLayout->addWidget(d->replyToUsernameLabel, 2, 0);
    internalLayout->addWidget(d->btnCancelReply, 2, 1);

    d->btnCancelReply->hide();
    d->replyToUsernameLabel->hide();
}

ComposerWidget::~ComposerWidget()
{
    delete d;
}

void ComposerWidget::setEditor(TextEdit *editor)
{
    qCDebug(CHOQOK);
    if (d->editor) {
        d->editor->deleteLater();
    }
    d->editor = editor;
    qCDebug(CHOQOK);
    if (d->editor) {
        QGridLayout *internalLayout = qobject_cast<QGridLayout *>(d->editorContainer->layout());
        internalLayout->addWidget(d->editor, 0, 0);
        connect(d->editor, &TextEdit::returnPressed, this, &ComposerWidget::submitPost);
        connect(d->editor, &TextEdit::textChanged, this, &ComposerWidget::editorTextChanged);
        connect(d->editor, &TextEdit::cleared, this, &ComposerWidget::editorCleared);
        editorTextChanged();
    } else {
        qCDebug(CHOQOK) << "Editor is NULL!";
    }
}

void ComposerWidget::setText(const QString &text, const QString &replyToId, const QString &replyToUsername)
{
    d->editor->prependText(text);
    this->replyToId = replyToId;
    this->replyToUsername = replyToUsername;
    if (!replyToUsername.isEmpty()) {
        d->replyToUsernameLabel->setText(i18n("Replying to <b>%1</b>", replyToUsername));
        d->btnCancelReply->show();
        d->replyToUsernameLabel->show();
    }
    d->editor->setFocus();
}

void ComposerWidget::submitPost(const QString &txt)
{
    qCDebug(CHOQOK);
    editorContainer()->setEnabled(false);
    QString text = txt;
    if (currentAccount()->postCharLimit() &&
            text.size() > (int)currentAccount()->postCharLimit()) {
        text = Choqok::ShortenManager::self()->parseText(text);
    }
    delete d->postToSubmit;
    d->postToSubmit = new Choqok::Post;
    d->postToSubmit->content = text;
    if (!replyToId.isEmpty()) {
        d->postToSubmit->replyToPostId = replyToId;
    }
    connect(d->currentAccount->microblog(), &MicroBlog::postCreated,
            this, &ComposerWidget::slotPostSubmited);
    connect(d->currentAccount->microblog(), &MicroBlog::errorPost,
            this, &ComposerWidget::slotErrorPost);
    btnAbort = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("Abort"), this);
    layout()->addWidget(btnAbort);
    connect(btnAbort, &QPushButton::clicked, this, &ComposerWidget::abort);
    currentAccount()->microblog()->createPost(currentAccount(), d->postToSubmit);
}

void ComposerWidget::slotPostSubmited(Choqok::Account *theAccount, Choqok::Post *post)
{
    qCDebug(CHOQOK);
    if (currentAccount() == theAccount && post == d->postToSubmit) {
        qCDebug(CHOQOK) << "Accepted";
        disconnect(d->currentAccount->microblog(), &MicroBlog::postCreated,
                   this, &ComposerWidget::slotPostSubmited);
        disconnect(d->currentAccount->microblog(), &MicroBlog::errorPost,
                   this, &ComposerWidget::slotErrorPost);
        if (btnAbort) {
            btnAbort->deleteLater();
        }
        d->editor->clear();
        editorCleared();
        editorContainer()->setEnabled(true);
        delete d->postToSubmit;
        d->postToSubmit = nullptr;
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void ComposerWidget::slotErrorPost(Account *theAccount, Post *post)
{
    qCDebug(CHOQOK);
    if (theAccount == d->currentAccount && post == d->postToSubmit) {
        qCDebug(CHOQOK);
        disconnect(d->currentAccount->microblog(), &MicroBlog::postCreated,
                   this, &ComposerWidget::slotPostSubmited);
        disconnect(d->currentAccount->microblog(), &MicroBlog::errorPost,
                   this, &ComposerWidget::slotErrorPost);
        if (btnAbort) {
            btnAbort->deleteLater();
        }
        editorContainer()->setEnabled(true);
        editor()->setFocus();
    }
}

void ComposerWidget::editorTextChanged()
{
    if (d->editor->toPlainText().length()) {
        d->editor->setMaximumHeight(qMax(d->editor->fontMetrics().height() * 3,
                                         80));
        d->editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        d->editor->setMaximumHeight(qMax(d->editor->fontMetrics().height(),
                                         30));
        d->editor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

TextEdit *ComposerWidget::editor()
{
    return d->editor;
}

QWidget *ComposerWidget::editorContainer()
{
    return d->editorContainer;
}

Post *ComposerWidget::postToSubmit()
{
    return d->postToSubmit;
}

void ComposerWidget::setPostToSubmit(Post *post)
{
    delete d->postToSubmit;
    d->postToSubmit = post;
}

QPointer< QPushButton > ComposerWidget::btnCancelReply()
{
    return d->btnCancelReply;
}

Account *ComposerWidget::currentAccount()
{
    return d->currentAccount;
}

QPointer< QLabel > ComposerWidget::replyToUsernameLabel()
{
    return d->replyToUsernameLabel;
}

void ComposerWidget::editorCleared()
{
    replyToId.clear();
    replyToUsername.clear();
    d->btnCancelReply->hide();
    d->replyToUsernameLabel->hide();
}

void ComposerWidget::abort()
{
    if (btnAbort) {
        btnAbort->deleteLater();
    }
    editorContainer()->setEnabled(true);
    currentAccount()->microblog()->abortCreatePost(currentAccount(), d->postToSubmit);
    editor()->setFocus();
}

}
}
