/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

namespace Choqok {
namespace UI {

class ComposerWidget::Private
{
public:
    Private( Account *account, TextEdit *editW = 0 )
    :editor(editW), currentAccount(account),btnAbort(0), postToSubmit(0)
    {}
    TextEdit *editor;
    Account *currentAccount;
    QString replyToId;
    QPointer<KPushButton> btnAbort;
    Choqok::Post *postToSubmit;
};

ComposerWidget::ComposerWidget(Choqok::Account* account, QWidget* parent /*= 0*/)
: QWidget(parent), d(new Private(account, new TextEdit(account->microblog()->postCharLimit(), this)))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->editor);
    connect(d->editor, SIGNAL(returnPressed(QString)), SLOT(submitPost(QString)));
    connect(d->editor, SIGNAL(textChanged()), SLOT(editorTextChanged()));
    connect(d->editor, SIGNAL(cleared()), SLOT(editorCleared()));
    editorTextChanged();
}

ComposerWidget::~ComposerWidget()
{
    delete d;
}

void ComposerWidget::setText(const QString& text, const QString& replyToId)
{
    d->editor->prependText(text);
    d->replyToId = replyToId;
    d->editor->setFocus(Qt::OtherFocusReason);
}

void ComposerWidget::submitPost( const QString &txt )
{
    editor()->setEnabled(false);
    QString text = txt;
    if( currentAccount()->microblog()->postCharLimit() &&
       text.size() > (int)currentAccount()->microblog()->postCharLimit() )
        text = Choqok::ShortenManager::self()->parseText(text);
    delete d->postToSubmit;
    d->postToSubmit = new Choqok::Post;
    d->postToSubmit->content = text;
    if( !d->replyToId.isEmpty() ) {
        d->postToSubmit->replyToPostId = d->replyToId;
    }
    connect(d->currentAccount->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
            SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)) );
    connect(d->currentAccount->microblog(),
            SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                             QString,Choqok::MicroBlog::ErrorLevel)),
            SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
    currentAccount()->microblog()->createPost( currentAccount(),d->postToSubmit);
    d->btnAbort = new KPushButton(KIcon("dialog-cancel"), i18n("Abort"), this);
    layout()->addWidget(d->btnAbort);
    connect( d->btnAbort, SIGNAL(clicked(bool)), SLOT(abort()) );
}

void ComposerWidget::slotPostSubmited(Choqok::Account* theAccount, Choqok::Post* post)
{
    if( currentAccount() == theAccount && post == d->postToSubmit ) {
        kDebug()<<"Accepted";
        disconnect(d->currentAccount->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
                    this, SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)) );
        disconnect(d->currentAccount->microblog(),
                    SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                             QString,Choqok::MicroBlog::ErrorLevel)),
                    this, SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
        if(d->btnAbort){
            d->btnAbort->deleteLater();
        }
        NotifyManager::success(i18n("New post submitted successfully"));
        d->editor->clear();
        d->replyToId.clear();
        d->editor->setEnabled(true);
        delete d->postToSubmit;
        d->postToSubmit = 0L;
        currentAccount()->microblog()->updateTimelines(currentAccount());
    }
}

void ComposerWidget::slotErrorPost(Account* theAccount, Post* post)
{
    if(theAccount == d->currentAccount && post == d->postToSubmit) {
        kDebug();
        disconnect(d->currentAccount->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
                   this, SLOT(slotPostSubmited(Choqok::Account*,Choqok::Post*)) );
        disconnect(d->currentAccount->microblog(),
                   SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,
                          QString,Choqok::MicroBlog::ErrorLevel)),
                   this, SLOT(slotErrorPost(Choqok::Account*,Choqok::Post*)));
        if(d->btnAbort){
            d->btnAbort->deleteLater();
        }
        editor()->setEnabled(true);
        editor()->setFocus();
    }
}

void ComposerWidget::editorTextChanged()
{
    if(d->editor->toPlainText().length())
        d->editor->setMaximumHeight(80);
    else
        d->editor->setMaximumHeight(30);
}

TextEdit* ComposerWidget::editor()
{
    return d->editor;
}

Account* ComposerWidget::currentAccount()
{
    return d->currentAccount;
}

void ComposerWidget::editorCleared()
{
    d->replyToId.clear();
}

void ComposerWidget::abort()
{
    if(d->btnAbort){
        d->btnAbort->deleteLater();
    }
    editor()->setEnabled(true);
    currentAccount()->microblog()->abortCreatePost(currentAccount(), d->postToSubmit);
    editor()->setFocus();
}

}
}
#include "composerwidget.moc"
