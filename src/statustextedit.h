/*
    This file is part of choqoK, the KDE micro-blogging client

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
    StatusTextEdit( QWidget *parent = 0 );

    ~StatusTextEdit();
    QStringList friendsList() const;
    int countOfRemainsChar() const;

public slots:
    void setDefaultDirection( Qt::LayoutDirection dir );
    void setNumOfCharsLeft();
    void clearContentsAndSetDirection( Qt::LayoutDirection dir );

protected:
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void insertFromMimeData ( const QMimeData *source );

signals:
    void returnPressed( QString &txt );
    void charsLeft( int count );
    void cleared();
private:
    QString prevStr;
    Qt::LayoutDirection dir;
    QStringList mFriendsList;
    int mCountOfRemainsChars;
};

#endif
