/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#include "statustextedit.h"
#include <QKeyEvent>
#include <KDE/KLocale>

StatusTextEdit::StatusTextEdit(QWidget *parent)
 : KTextEdit(parent)
{
	this->setAcceptRichText(false);
	connect(this, SIGNAL(textChanged()), this, SLOT(setNumOfCharsLeft()));
	setAcceptRichText(false);
    this->setToolTip(i18n("Press Ctrl+P to have previous text submitted.\nPress Ctrl+S to enable/disable spell checking."));
}

StatusTextEdit::~StatusTextEdit()
{
}

void StatusTextEdit::keyPressEvent(QKeyEvent * e)
{
	if ((e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Enter)) {
		QString txt = toPlainText();
		emit returnPressed(txt);
		e->accept();
    } else if(e->modifiers()==Qt::ControlModifier && e->key() == Qt::Key_S){
        this->setCheckSpellingEnabled( !this->checkSpellingEnabled() );
    } else if(e->modifiers()==Qt::ControlModifier && e->key() == Qt::Key_P){
        QString tmp = this->toHtml();
		this->setHtml(tmp+ ' ' +prevStr);
    } else if(e->key() == Qt::Key_Escape){
        if(!this->toPlainText().isEmpty()){
            this->clear();
            setDefaultDirection(dir);
            e->accept();
        } else {
            KTextEdit::keyPressEvent(e);
        }
    } else{
		KTextEdit::keyPressEvent(e);
	}
}

void StatusTextEdit::setDefaultDirection(Qt::LayoutDirection dir)
{
	QTextCursor c = this->textCursor();
	QTextBlockFormat f = c.blockFormat();
	f.setLayoutDirection(dir);
	c.setBlockFormat(f);
	this->setTextCursor(c);
	this->setFocus(Qt::OtherFocusReason);
}

void StatusTextEdit::setNumOfCharsLeft()
{
	int remainChar = 140 - toPlainText().count();
	emit charsLeft(remainChar);
}

void StatusTextEdit::clearContentsAndSetDirection(Qt::LayoutDirection dir)
{
	QString tmp = this->toPlainText();
    if(!tmp.isEmpty()){
        prevStr = this->toHtml();
        this->clear();
    }
	setDefaultDirection(dir);
}

/*
QStringList StatusTextEdit::friendsList() const
{
    return mFriendsList;
}

void StatusTextEdit::setFriendsList(const QStringList & list)
{
    mFriendsList = list;
}

void StatusTextEdit::clearFriendsList()
{
    mFriendsList.clear();
}

void StatusTextEdit::appendToFriendsList(const QStringList & list)
{
    mFriendsList<<list;
}
*/

#include "statustextedit.moc"


