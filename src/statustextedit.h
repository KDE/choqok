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
    QStringList friendsList() const;
//     void clearFriendsList();
//     void setFriendsList(const QStringList &list);
//     void appendToFriendsList(const QStringList &list);
public slots:
	void setDefaultDirection(Qt::LayoutDirection dir);
	void setNumOfCharsLeft();
	void clearContentsAndSetDirection(Qt::LayoutDirection dir);
	
protected:
	virtual void keyPressEvent(QKeyEvent *e);
	
signals:
	void returnPressed(QString &txt);
	void charsLeft(int count);
private:
	QString prevStr;
    Qt::LayoutDirection dir;
    QStringList mFriendsList;
};

#endif
