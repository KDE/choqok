/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "textbrowser.h"

#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QMenu>
#include <QMimeData>
#include <QPointer>
#include <QTextBrowser>

#include <KLocalizedString>

#include "postwidget.h"

using namespace Choqok::UI;

class TextBrowser::Private
{
public:
    Private()
        : isPressedForDrag(false)
    {}
    static QList< QPointer<QAction> > actions;
    PostWidget *parent;
    QPoint dragStartPosition;
    bool isPressedForDrag;
};

QList< QPointer<QAction> > TextBrowser::Private::actions;

TextBrowser::TextBrowser(QWidget *parent)
    : QTextBrowser(parent), d(new Private)
{
    d->parent = qobject_cast<PostWidget *>(parent);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setOpenLinks(false);
}

TextBrowser::~TextBrowser()
{
    delete d;
}

void TextBrowser::mousePressEvent(QMouseEvent *ev)
{
    Q_EMIT clicked(ev);

    if (ev->button() == Qt::LeftButton) {
        if (!cursorForPosition(ev->pos()).hasSelection() && !anchorAt(ev->pos()).isEmpty()) {
            d->dragStartPosition = ev->pos();
            d->isPressedForDrag = true;
        } else {
            d->isPressedForDrag = false;
        }
    }
    ev->accept();
    QTextBrowser::mousePressEvent(ev);
}

void TextBrowser::mouseMoveEvent(QMouseEvent *ev)
{
    if ((ev->buttons() & Qt::LeftButton) && d->isPressedForDrag) {
        QPoint diff = ev->pos() - d->dragStartPosition;
        if (diff.manhattanLength() > QApplication::startDragDistance()) {
            QString anchor = anchorAt(d->dragStartPosition);
            if (!anchor.isEmpty()) {
                QDrag *drag = new QDrag(this);
                QMimeData *mimeData;
                mimeData = new QMimeData;
                QList<QUrl> urls;
                urls.append(QUrl(anchor));
                mimeData->setUrls(urls);
                mimeData->setText(anchor);
                drag->setMimeData(mimeData);
                drag->exec(Qt::CopyAction | Qt::MoveAction);
            }
        } else {
            QTextBrowser::mouseMoveEvent(ev);
        }
    } else {
        QTextBrowser::mouseMoveEvent(ev);
    }
    ev->accept();
}

void TextBrowser::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);
}

void TextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    QAction *copy = new QAction(i18nc("Copy text", "Copy"), this);
//     copy->setShortcut( KShortcut( Qt::ControlModifier | Qt::Key_C ) );
    connect(copy, SIGNAL(triggered(bool)), SLOT(slotCopyPostContent()));
    menu->addAction(copy);
    QString anchor = document()->documentLayout()->anchorAt(event->pos());
    if (!anchor.isEmpty()) {
        QAction *copyLink = new QAction(i18n("Copy Link Location"), this);
        copyLink->setData(anchor);
        connect(copyLink, SIGNAL(triggered(bool)), SLOT(slotCopyLink()));
        menu->addAction(copyLink);
    }
    QAction *selectAll = new QAction(i18nc("Select all text", "Select All"), this);
//     selectAll->setShortcut( KShortcut( Qt::ControlModifier | Qt::Key_A ) );
    connect(selectAll, SIGNAL(triggered(bool)), SLOT(selectAll()));
    menu->addAction(selectAll);
    menu->addSeparator();
    Q_FOREACH (QAction *act, d->actions) {
        if (act) {
            act->setUserData(32, new PostWidgetUserData(d->parent));
            menu->addAction(act);
        }
    }
    menu->popup(event->globalPos());
}

void TextBrowser::slotCopyPostContent()
{
    QString txt = textCursor().selectedText();
    if (txt.isEmpty()) {
        PostWidget *paPost = qobject_cast<PostWidget *>(parentWidget());
        if (paPost) {
            QApplication::clipboard()->setText(paPost->currentPost()->content);
        }
    } else {
        QApplication::clipboard()->setText(txt);
    }
}

void TextBrowser::slotCopyLink()
{
    QAction *act = qobject_cast< QAction * >(sender());
    if (act) {
        QString link = act->data().toString();
        QApplication::clipboard()->setText(link);
    }
}

void Choqok::UI::TextBrowser::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}

void Choqok::UI::TextBrowser::addAction(QAction *action)
{
    if (action) {
        Private::actions.append(action);
    }
}

