/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <KDE/KTextBrowser>
#include "choqok_export.h"

class KAction;
namespace Choqok {

namespace UI {

class CHOQOK_EXPORT TextBrowser : public KTextBrowser
{
Q_OBJECT
public:
    TextBrowser(QWidget* parent = 0);
    virtual ~TextBrowser();
    static void addAction( KAction *action);

Q_SIGNALS:
    void clicked(QMouseEvent* ev);

protected Q_SLOTS:
    void slotCopyLink();

    /**
        @brief Copy post text to clipboard.
    */
    void slotCopyPostContent();

protected:
    virtual void wheelEvent(QWheelEvent* event);
    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void mouseMoveEvent(QMouseEvent* ev);
    virtual void resizeEvent(QResizeEvent* e);
    virtual void contextMenuEvent(QContextMenuEvent* event);

private:
    class Private;
    Private * const d;
};

}

}

#endif // CHOQOK_UI_TEXTBROWSER_H
