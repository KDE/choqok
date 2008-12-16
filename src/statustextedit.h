//
// C++ Interface: statustextedit
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSTEXTEDIT_H
#define STATUSTEXTEDIT_H

#include <ktextedit.h>

/**
	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class StatusTextEdit : public KTextEdit
{
	Q_OBJECT
public:
    StatusTextEdit(QWidget *parent=0);

    ~StatusTextEdit();
	
public slots:
	void setDefaultDirection(Qt::LayoutDirection dir);
	void setNumOfCharsLeft();
	void clearContentsAndSetDirection(Qt::LayoutDirection dir);
	
protected:
	virtual void keyPressEvent(QKeyEvent *e);
	
signals:
	void returnPressed(QString &txt);
	void charsLeft(int count);
// 	void aborted();
};

#endif
