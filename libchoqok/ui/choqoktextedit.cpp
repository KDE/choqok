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
#include "choqoktextedit.h"
#include <KLocale>
#include <QKeyEvent>
#include "shortenmanager.h"
#include <QGridLayout>
#include <QLabel>
#include <KDebug>
#include <choqokbehaviorsettings.h>

namespace Choqok {
namespace UI{

class TextEdit::Private
{
public:
    Private(uint charLmt)
    :charLimit(charLmt)
    {}
    uint charLimit;
    QString prevStr;
    QChar firstChar;
};

TextEdit::TextEdit(uint charLimit /*= 0*/, QWidget* parent /*= 0*/)
    :KTextEdit(parent), d(new Private(charLimit))
{
    kDebug()<<charLimit;
    connect( this, SIGNAL( textChanged() ), this, SLOT( updateRemainingCharsCount() ) );
    setAcceptRichText( false );
    this->setToolTip( i18n( "<strong>Note:</strong><br/><em>Ctrl+P</em> to have the previous submitted text.<br/>\
    <em>Ctrl+S</em> to enable/disable auto spell checker." ) );

    QFont counterF;
    counterF.setBold( true );
    counterF.setPointSize( 10 );
    lblRemainChar = new QLabel(this);
    lblRemainChar->resize(50,50);
    lblRemainChar->setFont( counterF );
    QGridLayout *layout = new QGridLayout(this);
    layout->setRowStretch(0,100);
    layout->setColumnStretch(5,100);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(lblRemainChar, 1, 0);
    this->setLayout(layout);
    setTabChangesFocus(true);
    settingsChanged();
    connect(BehaviorSettings::self(), SIGNAL(configChanged()), SLOT(settingsChanged()) );
}

TextEdit::~TextEdit()
{
    delete d;
}

void TextEdit::keyPressEvent(QKeyEvent* e)
{
    if (( e->key() == Qt::Key_Return ) || ( e->key() == Qt::Key_Enter ) ) {
        QString txt = toPlainText();
        emit returnPressed( txt );
        e->accept();
    } else if ( e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_S ) {
        this->setCheckSpellingEnabled( !this->checkSpellingEnabled() );
        e->accept();
    } else if ( e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_P ) {
        QString tmp = this->toHtml();
        this->setHtml( tmp + ' ' + d->prevStr );
        e->accept();
    } else if ( e->key() == Qt::Key_Escape ) {
        if ( !this->toPlainText().isEmpty() ) {
            this->clear();
            emit cleared();
            e->accept();
        } else {
            KTextEdit::keyPressEvent( e );
        }
    } else {
        KTextEdit::keyPressEvent( e );
    }
}

void TextEdit::clear()
{
    if(toPlainText().isEmpty())
        return;
    else {
        d->prevStr = toPlainText();
        KTextEdit::clear();
        emit cleared();
    }
}

void TextEdit::insertFromMimeData ( const QMimeData *source )
{
    if( Choqok::BehaviorSettings::shortenOnPaste() )
        KTextEdit::insertPlainText( ShortenManager::self()->parseText( source->text() ) );
    else
        KTextEdit::insertPlainText( source->text() );
}

void TextEdit::updateRemainingCharsCount()
{
    QString txt = this->toPlainText();
    int count = txt.count();
    if(count){
        lblRemainChar->show();
        if(d->charLimit){
            int remain = d->charLimit - count;
            if( remain < 0 ){
                lblRemainChar->setStyleSheet( "QLabel {color: red;}" );
            } else if(remain < 30) {
                lblRemainChar->setStyleSheet( "QLabel {color: rgb(242, 179, 19);}" );
            } else {
                lblRemainChar->setStyleSheet( "QLabel {color: green;}" );
            }
            lblRemainChar->setText( QString::number(remain) );
        } else {
            lblRemainChar->setText( QString::number(count) );
            lblRemainChar->setStyleSheet( "QLabel {color: blue;}" );
        }
        txt.remove(QRegExp("@([^\\s\\W]+)"));
        txt = txt.trimmed();
        if( d->firstChar != txt[0] ) {
            d->firstChar = txt[0];
            txt.prepend(' ');
            QTextBlockFormat f;
            f.setLayoutDirection( (Qt::LayoutDirection) txt.isRightToLeft() );
            textCursor().mergeBlockFormat( f );
        }
    }else{
        lblRemainChar->hide();
    }
}

void TextEdit::setCharLimit(uint charLimit /*= 0*/)
{
    d->charLimit = charLimit;
    updateRemainingCharsCount();
}

void TextEdit::setPlainText(const QString& text)
{
    KTextEdit::setPlainText(text);
    moveCursor(QTextCursor::End);
    setEnabled(true);
}

void TextEdit::setText(const QString& text)
{
    KTextEdit::setText(text);
    moveCursor(QTextCursor::End);
    setEnabled(true);
}

void TextEdit::prependText(const QString& text)
{
    QString tmp = text;
    tmp.append(' '+toPlainText());
    setPlainText(tmp);
}

void TextEdit::settingsChanged()
{
    setCheckSpellingEnabled(BehaviorSettings::enableSpellChecker());
}

}
}
#include "choqoktextedit.moc"
