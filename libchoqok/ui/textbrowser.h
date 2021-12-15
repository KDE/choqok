/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QTextBrowser>

#include "choqok_export.h"

class QAction;
namespace Choqok
{

namespace UI
{

class CHOQOK_EXPORT TextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    TextBrowser(QWidget *parent = nullptr);
    virtual ~TextBrowser();
    static void addAction(QAction *action);

Q_SIGNALS:
    void clicked(QMouseEvent *ev);

protected Q_SLOTS:
    void slotCopyLink();

    /**
        @brief Copy post text to clipboard.
    */
    void slotCopyPostContent();

protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseMoveEvent(QMouseEvent *ev) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private:
    class Private;
    Private *const d;
};

}

}

#endif // CHOQOK_UI_TEXTBROWSER_H
