/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mytextbrowser.h"

#include <QMouseEvent>

MyTextBrowser::MyTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
}

MyTextBrowser::~MyTextBrowser()
{
}

void MyTextBrowser::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);
    Q_EMIT mouseEntered();
}

void MyTextBrowser::leaveEvent(QEvent *e)
{
    QWidget::leaveEvent(e);
    Q_EMIT mouseLeaved();
}

void MyTextBrowser::mousePressEvent(QMouseEvent *ev)
{
    if (anchorAt(ev->pos()).isEmpty()) {
        Q_EMIT clicked();
    }
    QTextBrowser::mousePressEvent(ev);
}

