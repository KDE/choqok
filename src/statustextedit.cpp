//
// C++ Implementation: statustextedit
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "statustextedit.h"
#include <QKeyEvent>
StatusTextEdit::StatusTextEdit(QWidget *parent)
 : KTextEdit(parent)
{
	this->setAcceptRichText(false);
	connect(this, SIGNAL(textChanged()), this, SLOT(setNumOfCharsLeft()));
// 	setCheckSpellingEnabled(true);
	setAcceptRichText(false);
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
	} else{
		QTextEdit::keyPressEvent(e);
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
	this->clear();
	setDefaultDirection(dir);
}

#include "statustextedit.moc"


