/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
