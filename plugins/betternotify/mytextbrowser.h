/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2011  Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef MYTEXTBROWSER_H
#define MYTEXTBROWSER_H

#include <KTextBrowser>


class MyTextBrowser : public KTextBrowser
{
Q_OBJECT
public:
    MyTextBrowser(QWidget* parent = 0);
    virtual ~MyTextBrowser();

signals:
    void mouseEntered();
    void mouseLeaved();
    void clicked();

protected:
    virtual void enterEvent(QEvent* e);
    virtual void leaveEvent(QEvent* e);
    virtual void mousePressEvent(QMouseEvent* ev);
};

#endif // MYTEXTBROWSER_H
