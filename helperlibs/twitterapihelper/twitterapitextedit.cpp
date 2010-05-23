/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "twitterapitextedit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <KDebug>

class TwitterApiTextEdit::Private
{
public:
    Private()
    :c(0)
    {}
    QCompleter *c;
};

TwitterApiTextEdit::TwitterApiTextEdit(uint charLimit, QWidget* parent)
: TextEdit(charLimit, parent), d(new Private)
{
    kDebug();
    setTabChangesFocus(false);
}

TwitterApiTextEdit::~TwitterApiTextEdit()
{}

void TwitterApiTextEdit::setCompleter(QCompleter *completer)
{
    if (d->c)
        QObject::disconnect(d->c, 0, this, 0);

    d->c = completer;

    if (!d->c)
        return;

    d->c->setWidget(this);
    d->c->setCompletionMode(QCompleter::PopupCompletion);
    d->c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(d->c, SIGNAL(activated(const QString&)),
                     this, SLOT(insertCompletion(const QString&)));
}

QCompleter *TwitterApiTextEdit::completer() const
{
    return d->c;
}

void TwitterApiTextEdit::insertCompletion(const QString& completion)
{
    if (d->c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - d->c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString TwitterApiTextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TwitterApiTextEdit::focusInEvent(QFocusEvent *e)
{
    if (d->c)
        d->c->setWidget(this);
    QTextEdit::focusInEvent(e);
}

void TwitterApiTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (d->c && d->c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
//             case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
            default:
//                 Choqok::UI::TextEdit::keyPressEvent(e);
                break;
        }
    }

//     bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+E
//     if (!d->c )// || !isShortcut) // dont process the shortcut when we have a completer
    Choqok::UI::TextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier |
                                                Qt::MetaModifier);
    if (!d->c || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
//     bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if ( !e->text().isEmpty() && ( completionPrefix.length() < 2
        || eow.contains(e->text().right(1)) || !completionPrefix.startsWith('@') ) ) {
        d->c->popup()->hide();
        return;
    } else {
        if (completionPrefix.startsWith('@'))
            completionPrefix.remove(0, 1);
        if (completionPrefix != d->c->completionPrefix()) {
            d->c->setCompletionPrefix(completionPrefix);
            d->c->popup()->setCurrentIndex(d->c->completionModel()->index(0, 0));
        }
        QRect cr = cursorRect();
        cr.setWidth(d->c->popup()->sizeHintForColumn(0)
        + d->c->popup()->verticalScrollBar()->sizeHint().width());
        d->c->complete(cr); // popup it up!
    }
}

