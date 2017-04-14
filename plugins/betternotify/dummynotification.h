/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef DUMMYNOTIFICATION_H
#define DUMMYNOTIFICATION_H

#include <QTextBrowser>

class DummyNotification : public QTextBrowser
{
    Q_OBJECT
public:
    DummyNotification(const QFont &font, const QColor &color, const QColor &background, QWidget *parent);
    virtual ~DummyNotification();

protected:
    virtual void mouseMoveEvent(QMouseEvent *ev) override;
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;

Q_SIGNALS:
    void positionSelected(QPoint position);

protected Q_SLOTS:
    void slotProcessAnchor(const QUrl &url);

private:
    QPoint lastPressedPosition;
    bool isMoving;
};

#endif // DUMMYNOTIFICATION_H
