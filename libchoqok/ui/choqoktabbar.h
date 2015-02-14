/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2011-2012 Bardia Daneshvar <bardia.daneshvar@gmail.com>

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

#ifndef CHOQOKTABBAR_H
#define CHOQOKTABBAR_H

#include <QIcon>
#include <QString>
#include <QWidget>

#include "choqok_export.h"

namespace Choqok {

namespace UI {
  
class ChoqokTabBarPrivate;
class CHOQOK_EXPORT ChoqokTabBar : public QWidget
{
    Q_OBJECT
public:
    ChoqokTabBar( QWidget *parent = 0 );
    ~ChoqokTabBar();

    enum TabPosition {
                       North,
                       South,
                       West,
                       East
                     };

    enum SelectionBehavior {
                       SelectLeftTab,
                       SelectRightTab,
                       SelectPreviousTab
                     };

    enum ExtraWidgetPosition {
                       Top,
                       Bottom,
                       Left,
                       Right
                     };

    void setTabPosition( TabPosition position );
    TabPosition tabPosition() const;

    void setTabsClosable( bool closeable );
    bool tabsClosable() const;

    void setCornerWidget( QWidget *w , Qt::Corner corner = Qt::TopRightCorner );
    QWidget *cornerWidget( Qt::Corner corner = Qt::TopRightCorner ) const;

    void setExtraWidget( QWidget *widget , ExtraWidgetPosition position );
    QWidget *extraWidget( ExtraWidgetPosition position );

    void setTabAlongsideWidget( QWidget *widget );
    QWidget *tabAlongsideWidget() const;

    void setTabCloseActivatePrevious( bool stt );
    bool tabCloseActivatePrevious() const;

    SelectionBehavior selectionBehaviorOnRemove() const;
    void setSelectionBehaviorOnRemove ( SelectionBehavior behavior );

    QWidget *currentWidget() const;
    QWidget *widget( int index ) const;

    int currentIndex() const;
    int indexOf( QWidget *widget ) const;

    int addTab( QWidget *widget , const QString & name );
    int addTab( QWidget *widget , const QIcon & icon , const QString & name );

    int insertTab( int index , QWidget *widget , const QString & name );
    int insertTab( int index , QWidget *widget , const QIcon & icon , const QString & name );

    void moveTab( int from , int to );
    void removeTab( int index );

    void removePage( QWidget *widget );

    void setTabIcon( int index , const QIcon & icon );
    QIcon tabIcon( int index ) const;

    void setTabText( int index , const QString & text );
    QString tabText( int index ) const;


    /*! With LinkedAllTabBars function, all linked tabbars 
     *  using unique Settings .
     */
    void setLinkedTabBar( bool stt );
    bool linkedTabBar() const;

    void setTabBarHidden( bool stt );
    bool isTabBarHidden() const;

    QSize iconSize() const;
    void setIconSize( const QSize & size );

    int count() const;
    Qt::ToolButtonStyle toolButtonStyle() const;

    void setStyledTabBar( bool stt );
    bool styledTabBar() const;

    void refreshTabBar();

public Q_SLOTS:
    void setCurrentIndex ( int index );
    void setCurrentWidget ( QWidget *widget );

    void setToolButtonStyle( Qt::ToolButtonStyle toolButtonStyle );

Q_SIGNALS:
    void currentChanged( int index );
    void tabCloseRequested( int index );
    void tabMoved( int from , int to );

    void contextMenu( const QPoint & point );
    void contextMenu( QWidget *widget , const QPoint & point );

    void tabPositionChanged( TabPosition position );
    void styledPanelSignal( bool stt );
    void iconSizeChanged( const QSize & size );

protected:
    void resizeEvent( QResizeEvent *event );
    void paintEvent( QPaintEvent *event );

private Q_SLOTS:
    void action_triggered( QAction *action );
    void contextMenuRequest( const QPoint & point );
    void widget_destroyed( QObject *obj );

private: // Private Functions
    void init_style();
    void init_position( TabPosition position );
    void init_extra_widget( const QSize & size );
    void init_alongside_widget( const QSize & size );

private:
    ChoqokTabBarPrivate *p;
};

}
}

#endif // CHOQOKTABBAR_H
