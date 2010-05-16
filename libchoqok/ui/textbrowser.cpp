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

#include "textbrowser.h"
#include <KMenu>
#include <KAction>
#include <KLocalizedString>
#include <QApplication>
#include <QClipboard>
#include "postwidget.h"
#include <QAbstractTextDocumentLayout>

using namespace Choqok::UI;

Choqok::UI::TextBrowser::TextBrowser(QWidget* parent)
    : KTextBrowser(parent, true)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setOpenLinks(false);

}

Choqok::UI::TextBrowser::~TextBrowser()
{

}

void TextBrowser::mousePressEvent(QMouseEvent* ev)
{
    emit clicked(ev);
    QTextBrowser::mousePressEvent(ev);
}

void TextBrowser::resizeEvent(QResizeEvent* e)
{
    QTextEdit::resizeEvent(e);
}

void TextBrowser::contextMenuEvent(QContextMenuEvent* event)
{
    KMenu *menu = new KMenu(this);
    KAction *copy = new KAction( i18n("Copy"), this );
//     copy->setShortcut( KShortcut( Qt::ControlModifier | Qt::Key_C ) );
    connect( copy, SIGNAL(triggered(bool)), SLOT(slotCopyPostContent()) );
    menu->addAction(copy);
    QString anchor = document()->documentLayout()->anchorAt(event->pos());
    if( !anchor.isEmpty() ){
        KAction *copyLink = new KAction( i18n("Copy Link Location"), this );
        copyLink->setData( anchor );
        connect( copyLink, SIGNAL(triggered(bool)), SLOT(slotCopyLink()) );
        menu->addAction(copyLink);
    }
    menu->addSeparator();
    KAction *selectAll = new KAction(i18n("Select All"), this);
//     selectAll->setShortcut( KShortcut( Qt::ControlModifier | Qt::Key_A ) );
    connect( selectAll, SIGNAL(triggered(bool)), SLOT(selectAll()) );
    menu->addAction(selectAll);
    menu->popup(event->globalPos());
}

void TextBrowser::slotCopyPostContent()
{
    QString txt = textCursor().selectedText();
    if( txt.isEmpty() ){
        PostWidget *paPost = qobject_cast<PostWidget*>(parentWidget());
        if(paPost)
            QApplication::clipboard()->setText( paPost->currentPost().content );
     } else {
        QApplication::clipboard()->setText( txt );
     }
}

void TextBrowser::slotCopyLink()
{
    KAction *act = qobject_cast< KAction* >( sender() );
    if( act ){
        QString link = act->data().toString();
        QApplication::clipboard()->setText( link );
    }
}


#include "textbrowser.moc"
