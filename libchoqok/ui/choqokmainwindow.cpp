/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "choqokmainwindow.h"
#include "choqokbehaviorsettings.h"
#include <kstatusbar.h>
#include <KTabWidget>
#include "microblogwidget.h"

#ifdef QTINDICATE_BUILD
#include "indicatormanager.h"
#endif

using namespace Choqok::UI;

static const int TIMEOUT = 5000;

Choqok::UI::MainWindow::MainWindow()
    :KXmlGuiWindow()
{
#ifdef QTINDICATE_BUILD
    Choqok::MessageIndicatorManager::self();
#endif
}

Choqok::UI::MainWindow::~MainWindow()
{

}

QSize MainWindow::sizeHint() const
{
    return QSize( 350, 400 );
}

void MainWindow::showStatusMessage( const QString &message, bool isPermanent )
{
    if ( isPermanent ) {
        statusBar()->showMessage( message );
    } else {
        statusBar()->showMessage( message, TIMEOUT );
    }
}

void MainWindow::hideEvent( QHideEvent * event )
{
    Q_UNUSED(event);
    if( !this->isVisible() ) {
        kDebug();
        if( Choqok::BehaviorSettings::markAllAsReadOnHideToSystray() )
            emit markAllAsRead();
        emit removeOldPosts();
    }
}

Choqok::UI::MicroBlogWidget* MainWindow::currentMicroBlog()
{
    return qobject_cast<Choqok::UI::MicroBlogWidget*>(mainWidget->currentWidget());
}

void Choqok::UI::MainWindow::activateChoqok()
{
    showNormal();
    activateWindow();
    raise();
}

QList<Choqok::UI::MicroBlogWidget*> Choqok::UI::MainWindow::microBlogsWidgetsList()
{
   QList<Choqok::UI::MicroBlogWidget*> lst;
   if ( mainWidget->currentWidget() )
      for ( int i = 0;i < mainWidget->count();i++ )
          lst.append( qobject_cast<Choqok::UI::MicroBlogWidget*>( mainWidget->widget( i ) ) );
   return lst;
}

void Choqok::UI::MainWindow::activateTab( int k )
{
   if ( mainWidget->count() >= k )
       mainWidget->setCurrentIndex( k );
}

#include "choqokmainwindow.moc"
