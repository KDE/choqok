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

#include "dummynotification.h"

#include <QMouseEvent>

#include "choqokdebug.h"
#include <QIcon>
#include <KLocalizedString>

#include "choqokappearancesettings.h"
#include "choqoktools.h"
#include "notifysettings.h"
#include "postwidget.h"

DummyNotification::DummyNotification(const QFont& font, const QColor& color, const QColor& background, QWidget* parent)
: KTextBrowser(parent), isMoving(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setWindowOpacity(0.8);
    setWindowFlags(Qt::ToolTip);
    setFrameShape(QFrame::NoFrame);
    setOpenExternalLinks(false);
    setOpenLinks(false);
    setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"), QIcon::fromTheme("choqok").pixmap(48) );
    document()->addResource( QTextDocument::ImageResource, QUrl("icon://close"),
                                        QIcon::fromTheme("dialog-ok").pixmap(16) );
    setText(baseText.arg(i18n("Choqok")).arg(i18n("KDE Rocks! :)")).arg(i18n("OK")));
    connect(this, SIGNAL(anchorClicked(QUrl)), SLOT(slotProcessAnchor(QUrl)));

    QString fntStr = "font-family:\"" + font.family() + "\"; font-size:" + QString::number(font.pointSize()) + "pt;";
    fntStr += (font.bold() ? " font-weight:bold;" : QString()) + (font.italic() ? " font-style:italic;" : QString());
    QString style = Choqok::UI::PostWidget::getBaseStyle().arg( Choqok::getColorString(color), Choqok::getColorString(background), fntStr);

    setStyleSheet( style );
}

DummyNotification::~DummyNotification()
{
}

void DummyNotification::mouseMoveEvent(QMouseEvent* ev)
{
    KTextBrowser::mouseMoveEvent(ev);
    if(isMoving){
        QPoint diff = ev->globalPos() - lastPressedPosition;
        lastPressedPosition = ev->globalPos();
        QPoint newPos = pos() + diff;
        move(newPos);
    }
}

void DummyNotification::mousePressEvent(QMouseEvent* ev)
{
    qCDebug(CHOQOK);
    KTextBrowser::mousePressEvent(ev);
    isMoving = true;
    lastPressedPosition = ev->globalPos();
}

void DummyNotification::mouseReleaseEvent(QMouseEvent* ev)
{
    KTextBrowser::mouseReleaseEvent(ev);
    qCDebug(CHOQOK);
    isMoving = false;
}

void DummyNotification::slotProcessAnchor(const QUrl& url)
{
    qCDebug(CHOQOK);
    if(url.scheme() == "choqok"){
        if(url.host() == "close"){
            Q_EMIT positionSelected(pos());
        }
    }
}
