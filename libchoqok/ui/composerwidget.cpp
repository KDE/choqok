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

namespace Choqok {
namespace UI {

class ComposerWidget::Private
{
public:
    Private( Account *account, TextEdit *editW = 0 )
    :editor(editW), currentAccount(account)
    {}
    TextEdit *editor;
    Account *currentAccount;
    QString replyToId;
};

ComposerWidget::ComposerWidget(Choqok::Account* account, QWidget* parent /*= 0*/)
: QWidget(parent), d(new Private(account, new TextEdit(account->microblog()->postCharLimit(), this)))
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(d->editor);
    connect( d->editor, SIGNAL(returnPressed(QString)), SLOT(submitPost(QString)));
    connect(d->currentAccount->microblog(), SIGNAL(postCreated(Account*,Post*)),
            SLOT(slotPostSubmited(Account*,Post*)) );
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
    d->editor->setPlainText(text);
    d->replyToId = replyToId;
    d->editor->setFocus(Qt::OtherFocusReason);
}

void ComposerWidget::submitPost( const QString &text )
{
    Choqok::Post *ps = new Choqok::Post;
    ps->content = text;
    if( !d->replyToId.isEmpty() ) {
        ps->replyToPostId = d->replyToId;
    }
    currentAccount()->microblog()->createPost( currentAccount(),ps);
    editor()->setEnabled(false);
}

void ComposerWidget::slotPostSubmited(Choqok::Account* theAccount, Choqok::Post* post)
{
    kDebug();
    if( currentAccount() == theAccount && post->content == d->editor->toPlainText() ) {
        kDebug()<<"Accepted";
        d->editor->clear();
        d->replyToId.clear();
        d->editor->setEnabled(true);
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

}
}
#include "composerwidget.moc"
