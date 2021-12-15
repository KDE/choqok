/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
