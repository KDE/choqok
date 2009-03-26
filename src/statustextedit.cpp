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
#include "statustextedit.h"
#include <QKeyEvent>
#include <KDE/KLocale>

#include "backend.h"
#include <settings.h>

StatusTextEdit::StatusTextEdit( QWidget *parent )
        : KTextEdit( parent )
{
    this->setAcceptRichText( false );
    connect( this, SIGNAL( textChanged() ), this, SLOT( setNumOfCharsLeft() ) );
    setAcceptRichText( false );
    this->setToolTip( i18n( "Press Ctrl+P to have the previous text submitted.\n\
Press Ctrl+S to enable/disable spell-checking." ) );
}

StatusTextEdit::~StatusTextEdit()
{
}

void StatusTextEdit::keyPressEvent( QKeyEvent * e )
{
    if (( e->key() == Qt::Key_Return ) || ( e->key() == Qt::Key_Enter ) ) {
        QString txt = toPlainText();
        emit returnPressed( txt );
        e->accept();
    } else if ( e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_S ) {
        this->setCheckSpellingEnabled( !this->checkSpellingEnabled() );
    } else if ( e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_P ) {
        QString tmp = this->toHtml();
        this->setHtml( tmp + ' ' + prevStr );
    } else if ( e->key() == Qt::Key_Escape ) {
        if ( !this->toPlainText().isEmpty() ) {
            this->clear();
            setDefaultDirection( dir );
            emit cleared();
            e->accept();
        } else {
            KTextEdit::keyPressEvent( e );
        }
    } else {
        KTextEdit::keyPressEvent( e );
    }
}

void StatusTextEdit::insertFromMimeData ( const QMimeData *source )
{
    if( Settings::shortenOnPaste() )
        KTextEdit::insertPlainText( Backend::prepareStatus( source->text() ) );
    else
        KTextEdit::insertPlainText( source->text() );
}

void StatusTextEdit::setDefaultDirection( Qt::LayoutDirection dir )
{
    QTextCursor c = this->textCursor();
    QTextBlockFormat f = c.blockFormat();
    f.setLayoutDirection( dir );
    c.setBlockFormat( f );
    this->setTextCursor( c );
    this->setFocus( Qt::OtherFocusReason );
}

void StatusTextEdit::setNumOfCharsLeft()
{
    mCountOfRemainsChars = 140 - toPlainText().count();
    emit charsLeft( mCountOfRemainsChars );
}

void StatusTextEdit::clearContentsAndSetDirection( Qt::LayoutDirection dir )
{
    QString tmp = this->toPlainText();
    if ( !tmp.isEmpty() ) {
        prevStr = this->toHtml();
        this->clear();
    }
    setDefaultDirection( dir );
}

int StatusTextEdit::countOfRemainsChar() const
{
    return mCountOfRemainsChars;
}

#include "statustextedit.moc"
