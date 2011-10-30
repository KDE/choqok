/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "composerwidget.h"
#include "choqoktextedit.h"
#include "account.h"
#include <microblog.h>
#include <QHBoxLayout>
#include <KDebug>
#include <notifymanager.h>
#include <KPushButton>
#include <shortenmanager.h>
#include <qpointer.h>
#include <QGridLayout>
#include <QLabel>

namespace Choqok {
namespace UI {

class ComposerWidget::Private
{
public:
    Private( Account *account)
    :editor(0), currentAccount(account), postToSubmit(0)
    {}
    QPointer<TextEdit> editor;
    Account *currentAccount;
    Choqok::Post *postToSubmit;
    QWidget *editorContainer;
    QPointer<QLabel> replyToUsernameLabel;
    QPointer<KPushButton> btnCancelReply;
};

ComposerWidget::ComposerWidget(Choqok::Account* account, QWidget* parent /*= 0*/)
: QWidget(parent), btnAbort(0), d(new Private(account))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    d->editorContainer = new QWidget(this);
    QGridLayout *internalLayout = new QGridLayout;
    internalLayout->setContentsMargins(0, 0, 0, 0);
    d->editorContainer->setLayout(internalLayout);
    layout->addWidget(editorContainer());
    setEditor(new TextEdit(account->microblog()->postCharLimit(), this));

    d->replyToUsernameLabel = new QLabel(editorContainer());
    d->btnCancelReply = new KPushButton(editorContainer());
    d->btnCancelReply->setIcon(KIcon("dialog-cancel"));
    d->btnCancelReply->setToolTip(i18n("Discard Reply"));
    d->btnCancelReply->setMaximumWidth(d->btnCancelReply->height());
    connect( d->btnCancelReply, SIGNAL(clicked(bool)), SLOT(editorCleared()) );
    internalLayout->addWidget(d->replyToUsernameLabel, 2, 0);
    internalLayout->addWidget(d->btnCancelReply, 2, 1);

    d->btnCancelReply->hide();
    d->replyToUsernameLabel->hide();
}

ComposerWidget::~ComposerWidget()
{
    delete d;
}

void ComposerWidget::setEditor(TextEdit* editor)
{
    kDebug();
    if(d->editor)
        d->editor->deleteLater();
    d->editor = editor;
    kDebug();
    if(d->editor) {
        QGridLayout *internalLayout = qobject_cast<QGridLayout*>(d->editorContainer->layout());
        internalLayout->addWidget(d->editor, 0, 0);
        connect(d->editor, SIGNAL(returnPressed(QString)), SLOT(submitPost(QString)));
        connect(d->editor, SIGNAL(textChanged()), SLOT(editorTextChanged()));
        connect(d->editor, SIGNAL(cleared()), SLOT(editorCleared()));
        editorTextChanged();
    } else {
        kDebug()<<"Editor is NULL!";
    }
}

void ComposerWidget::setText(const QString& text, const QString& replyToId, const QString& replyToUsername)
{
    d->editor->prependText(text);
    this->replyToId = replyToId;
    this->replyToUsername = replyToUsername;
    if( !replyToUsername.isEmpty() ){
        d->replyToUsernameLabel->setText(i18n("Replying to <b>%1</b>", replyToUsername));
        d->btnCancelReply->show();
        d->replyToUsernameLabel->show();
    }
    d->editor->setFocus();
}

void ComposerWidget::submitPost( const QString &txt )
{
    kDebug();
    editorContainer()->setEnabled(false);
    QString text = txt;
    if( currentAccount()->microblog()->postCharLimit() &&
       text.size() > (int)currentAccount()->microblog()->postCharLimit() )
        text = Choqok::ShortenManager::self()->parseText(text);
    delete d->postToSubmit;
    d->postToSubmit = new Choqok::Post;
    d->postToSubmit->content = text;
    if( !replyToId.isEmpty() ) {
        d->postToSubmit->replyToPostId = replyToId;
    }
    connect(d->currentAccount->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
            SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)) );
    connect(d->currentAccount->microblog(),
            SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                             QString,Choqok::MicroBlog::ErrorLevel)),
            SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
    btnAbort = new KPushButton(KIcon("dialog-cancel"), i18n("Abort"), this);
    layout()->addWidget(btnAbort);
    connect( btnAbort, SIGNAL(clicked(bool)), SLOT(abort()) );
    currentAccount()->microblog()->createPost( currentAccount(),d->postToSubmit);
}

void ComposerWidget::slotPostSubmited(Choqok::Account* theAccount, Choqok::Post* post)
{
    kDebug();
    if( currentAccount() == theAccount && post == d->postToSubmit ) {
        kDebug()<<"Accepted";
        disconnect(d->currentAccount->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
                    this, SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)) );
        disconnect(d->currentAccount->microblog(),
                    SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                             QString,Choqok::MicroBlog::ErrorLevel)),
                    this, SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
        if(btnAbort){
            btnAbort->deleteLater();
        }
        d->editor->clear();
        editorCleared();
        editorContainer()->setEnabled(true);
        delete d->postToSubmit;
        d->postToSubmit = 0L;
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void ComposerWidget::slotErrorPost(Account* theAccount, Post* post)
{
    kDebug();
    if(theAccount == d->currentAccount && post == d->postToSubmit) {
        kDebug();
        disconnect(d->currentAccount->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
                   this, SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)) );
        disconnect(d->currentAccount->microblog(),
                   SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                          QString,Choqok::MicroBlog::ErrorLevel)),
                   this, SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
        if(btnAbort){
            btnAbort->deleteLater();
        }
        editorContainer()->setEnabled(true);
        editor()->setFocus();
    }
}

void ComposerWidget::editorTextChanged()
{
    if(d->editor->toPlainText().length()) {
        d->editor->setMaximumHeight(80);
        d->editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    else {
        d->editor->setMaximumHeight(30);
        d->editor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

TextEdit* ComposerWidget::editor()
{
    return d->editor;
}

QWidget* ComposerWidget::editorContainer()
{
    return d->editorContainer;
}

Post* ComposerWidget::postToSubmit()
{
    return d->postToSubmit;
}

void ComposerWidget::setPostToSubmit(Post* post)
{
    delete d->postToSubmit;
    d->postToSubmit = post;
}

Account* ComposerWidget::currentAccount()
{
    return d->currentAccount;
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
    if(btnAbort){
        btnAbort->deleteLater();
    }
    editorContainer()->setEnabled(true);
    currentAccount()->microblog()->abortCreatePost(currentAccount(), d->postToSubmit);
    editor()->setFocus();
}

}
}
#include "composerwidget.moc"
