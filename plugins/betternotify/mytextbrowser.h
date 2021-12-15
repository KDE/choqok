/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MYTEXTBROWSER_H
#define MYTEXTBROWSER_H

#include <QTextBrowser>

class MyTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    MyTextBrowser(QWidget *parent = nullptr);
    virtual ~MyTextBrowser();

Q_SIGNALS:
    void mouseEntered();
    void mouseLeaved();
    void clicked();

protected:
    virtual void enterEvent(QEvent *e) override;
    virtual void leaveEvent(QEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *ev) override;
};

#endif // MYTEXTBROWSER_H
