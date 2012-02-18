/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "systrayicon.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <KDE/KLocale>
#include <QPainter>
#include <KColorScheme>
#include <QTimer>
#include "accountmanager.h"
#include <kglobalsettings.h>
#include <mediamanager.h>
#include <QWheelEvent>
#include <KDebug>

SysTrayIcon::SysTrayIcon( Choqok::UI::MainWindow* parent )
: KStatusNotifierItem( parent ), _mainwin(parent), isOffline(false)
{
    kDebug();
    unread = 0;
    setAssociatedWidget(parent);
    setCategory(ApplicationStatus);
    setStandardActionsEnabled(false);
//     setStatus(Active);
    setIconByName( currentIconName() );
}

SysTrayIcon::~SysTrayIcon()
{
    kDebug();
}

void SysTrayIcon::resetUnreadCount()
{
    updateUnreadCount(-unread);
}

QString SysTrayIcon::currentIconName()
{
    if(isOffline)
        return "choqok_offline";
    else
        return "choqok";
}

void SysTrayIcon::updateUnreadCount( int changeOfUnreadPosts )
{
    kDebug();
    unread += changeOfUnreadPosts;

    if ( unread <= 0 ) {
        setIconByName(currentIconName());
        unread = 0;
        setStatus(Passive);
    } else {
        setStatus(Active);
        int oldWidth = 22;

        QString countStr = QString::number( unread );
        QFont f = KGlobalSettings::generalFont();
        f.setBold( true );

        float pointSize = f.pointSizeF();
        QFontMetrics fm( f );
        int w = fm.width( countStr );
        if ( w > ( oldWidth - 2 ) ) {
            pointSize *= float( oldWidth - 2 ) / float( w );
            f.setPointSizeF( pointSize );
        }

        // overlay
        QPixmap overlayImg = KIcon(currentIconName()).pixmap(22,22);
        QPainter p( &overlayImg );
        p.setFont( f );
        KColorScheme scheme( QPalette::Active, KColorScheme::View );

        fm = QFontMetrics( f );
        QRect boundingRect = fm.tightBoundingRect( countStr );
        boundingRect.adjust( 0, 0, 0, 2 );
        boundingRect.setHeight( qMin( boundingRect.height(), oldWidth ) );
        boundingRect.moveTo(( oldWidth - boundingRect.width() ) / 2,
                            (( oldWidth - boundingRect.height() ) / 2 ) - 1 );
        p.setOpacity( 0.7 );
        QBrush br(QColor(255, 255, 255), Qt::SolidPattern);
        p.setBrush( br );
        p.setPen( QColor(255, 255, 255) );
        p.drawRoundedRect( boundingRect, 2.0, 2.0 );

        p.setBrush( Qt::NoBrush );
        p.setPen( QColor( 0, 0, 0 ) );
        p.setOpacity( 1.0 );
        p.drawText( overlayImg.rect(), Qt::AlignCenter, countStr );
        setIconByPixmap( overlayImg );
    }
    this->setToolTip( "choqok", i18n("Choqok"), i18np( "1 unread post", "%1 unread posts", unread ) );
}

void SysTrayIcon::setTimeLineUpdatesEnabled( bool isEnabled )
{
    if ( isEnabled ) {
        setToolTip( "choqok", i18n( "Choqok" ), QString() );
        setIconByName("choqok");
    } else {
        setToolTip( "choqok", i18n( "Choqok - Disabled" ), QString() );
        setIconByName("choqok_offline");
    }
    isOffline = !isEnabled;
    updateUnreadCount( 0 );
}

void SysTrayIcon::slotJobDone( Choqok::JobResult result )
{
    kDebug();
    if ( result == Choqok::Success ) {
        setOverlayIconByName( "task-complete" );
    } else {
        setOverlayIconByName( "task-reject" );
    }
    QTimer::singleShot( 5000, this, SLOT( slotRestoreIcon() ) );
}

void SysTrayIcon::slotRestoreIcon()
{
    setOverlayIconByName(QString());
    updateUnreadCount( 0 );
}

int SysTrayIcon::unreadCount() const
{
    return unread;
}

#include "systrayicon.moc"
