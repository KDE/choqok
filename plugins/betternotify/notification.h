/*
    This file is part of Choqok, the KDE micro-blogging client*
    Copyright (C) 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "postwidget.h"

#include "mytextbrowser.h"

class Notification : public QWidget
{
    Q_OBJECT
public:
    Notification(Choqok::UI::PostWidget *post);
    virtual ~Notification();
    void init();

    virtual QSize sizeHint() const override;

Q_SIGNALS:
    void ignored();
    void postReaded();
    void mouseEntered();
    void mouseLeaved();

protected Q_SLOTS:
    void slotProcessAnchor(const QUrl &url);
    void slotClicked();

protected:
    virtual void mouseMoveEvent(QMouseEvent *) override;

private:
    void setDirection();
    void setHeight();
    static const QRegExp dirRegExp;
    Choqok::UI::PostWidget *post;
    QString dir;
    MyTextBrowser mainWidget;
};

#endif // NOTIFICATION_H
