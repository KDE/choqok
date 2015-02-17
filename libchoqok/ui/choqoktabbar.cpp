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

#include "choqoktabbar.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QHash>
#include <QHBoxLayout>
#include <QList>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <KAction>
#include <KMenu>

#include "choqokappearancesettings.h"

#define ICON_SMALL_SIZE  22
#define ICON_MEDIUM_SIZE 32
#define ICON_BIG_SIZE    40

namespace Choqok {
namespace UI {

QList<ChoqokTabBar*> choqok_tabbars_list;

class ChoqokTabBarPrivate
{
public:
    QToolBar *toolbar;
    QStackedWidget *st_widget;
//     QWidget *tab_widget;
    QWidget *tab_alongside_widget;

    QGridLayout *main_layout;
    QGridLayout *stack_wgt_layout;

    ChoqokTabBar::TabPosition       position;
    ChoqokTabBar::SelectionBehavior selection_behavior;
    bool tab_closable;
    bool styled_tabbar;

    QHash<Qt::Corner,QWidget*> corners_hash;
    QHash<ChoqokTabBar::ExtraWidgetPosition,QWidget*> extra_wgt_hash;

    QList<QAction*> actions_list;
    QList<int> history_list;

    QPalette old_palette;
};


ChoqokTabBar::ChoqokTabBar(QWidget *parent) :
    QWidget(parent)
{
    p = new ChoqokTabBarPrivate;
    p->position = (TabPosition)Choqok::AppearanceSettings::tabBarPosition();
    p->styled_tabbar = Choqok::AppearanceSettings::tabBarIsStyled();
    p->tab_alongside_widget = 0;

//     p->tab_widget = new QWidget();
    p->st_widget = new QStackedWidget();
    p->toolbar = new QToolBar();
    p->toolbar->setContextMenuPolicy( Qt::CustomContextMenu );

    p->stack_wgt_layout = new QGridLayout();
    p->stack_wgt_layout->addWidget( p->st_widget , 1 , 1 );
    p->stack_wgt_layout->setContentsMargins( 0 , 0 , 0 , 0 );

    p->main_layout = new QGridLayout( this );
    p->main_layout->setSpacing( 0 );
    p->main_layout->setContentsMargins( 0 , 0 , 0 , 0 );
    p->main_layout->addLayout( p->stack_wgt_layout , 1 , 1 );

    connect( p->toolbar , SIGNAL(actionTriggered(QAction*))          , SLOT(action_triggered(QAction*)) );
    connect( p->toolbar , SIGNAL(customContextMenuRequested(QPoint)) , SLOT(contextMenuRequest(QPoint)) );

    setToolButtonStyle( Qt::ToolButtonIconOnly );
    int iconSize = Choqok::AppearanceSettings::tabBarSize();
    if(iconSize != ICON_BIG_SIZE && iconSize != ICON_MEDIUM_SIZE && iconSize != ICON_SMALL_SIZE)
        iconSize = ICON_MEDIUM_SIZE;
    
    init_position( p->position );
    setIconSize( QSize(iconSize,iconSize) );
    init_style();
}

void ChoqokTabBar::setTabPosition( ChoqokTabBar::TabPosition position )
{
    if( position == p->position )
        return;

    p->main_layout->removeWidget( p->toolbar );

    init_position( position );
    init_style();
    init_alongside_widget( size() );


    /*! ----------- Setting Up All Linked ChoqokTabBars ------------ */
    if( linkedTabBar() )
        for( int i=0 ; i<choqok_tabbars_list.count() ; i++ )
            choqok_tabbars_list.at(i)->setTabPosition( position );
    /*! ------------------------------------------------------------ */

    Q_EMIT tabPositionChanged( position );
}

void ChoqokTabBar::init_position( ChoqokTabBar::TabPosition position )
{
    p->position = position;

    /*! ---------- Adding to the New Layout -------------- */
    switch( static_cast<int>(position) )
    {
    case ChoqokTabBar::North :
        p->main_layout->addWidget( p->toolbar , 0 , 1 );
        p->toolbar->setOrientation( Qt::Horizontal );
        p->toolbar->setSizePolicy( QSizePolicy::MinimumExpanding , QSizePolicy::Minimum );
        break;

    case ChoqokTabBar::South :
        p->main_layout->addWidget( p->toolbar , 2 , 1 );
        p->toolbar->setOrientation( Qt::Horizontal );
        p->toolbar->setSizePolicy( QSizePolicy::MinimumExpanding , QSizePolicy::Minimum );
        break;

    case ChoqokTabBar::West :
        p->main_layout->addWidget( p->toolbar , 1 , 0 );
        p->toolbar->setOrientation( Qt::Vertical );
        p->toolbar->setSizePolicy( QSizePolicy::Minimum , QSizePolicy::MinimumExpanding );
        break;

    case ChoqokTabBar::East :
        p->main_layout->addWidget( p->toolbar , 1 , 2 );
        p->toolbar->setOrientation( Qt::Vertical );
        p->toolbar->setSizePolicy( QSizePolicy::Minimum , QSizePolicy::MinimumExpanding );
        break;
    }
}

ChoqokTabBar::TabPosition ChoqokTabBar::tabPosition() const
{
    return p->position;
}

void ChoqokTabBar::setTabsClosable( bool closeable )
{
    if( p->tab_closable == closeable )
      return;

    p->tab_closable = closeable;

    /*! ----------- Setting Up All Linked ChoqokTabBars ------------ */
    if( linkedTabBar() )
        for( int i=0 ; i<choqok_tabbars_list.count() ; i++ )
            choqok_tabbars_list.at(i)->setTabsClosable( closeable );
    /*! ------------------------------------------------------------ */
}

bool ChoqokTabBar::tabsClosable() const
{
    return p->tab_closable;
}

void ChoqokTabBar::setCornerWidget( QWidget *w , Qt::Corner corner )
{
    if( p->corners_hash.contains(corner) )
        return;

    p->corners_hash.insert( corner , w );
}

QWidget *ChoqokTabBar::cornerWidget( Qt::Corner corner ) const
{
    return p->corners_hash.value( corner );
}

void ChoqokTabBar::setExtraWidget( QWidget *widget , ChoqokTabBar::ExtraWidgetPosition position )
{
    if( p->extra_wgt_hash.contains(position) )
        p->extra_wgt_hash.remove( position );

    if( p->extra_wgt_hash.values().contains(widget) )
        p->extra_wgt_hash.remove( p->extra_wgt_hash.key(widget) );

    switch( static_cast<int>(position) )
    {
    case ChoqokTabBar::Top :
        p->stack_wgt_layout->addWidget( widget , 0 , 1 );
        break;

    case ChoqokTabBar::Bottom :
        p->stack_wgt_layout->addWidget( widget , 2 , 1 );
        break;

    case ChoqokTabBar::Left :
        p->stack_wgt_layout->addWidget( widget , 1 , 0 );
        break;

    case ChoqokTabBar::Right :
        p->stack_wgt_layout->addWidget( widget , 1 , 2 );
        break;
    }

    p->extra_wgt_hash.insert( position , widget );

    init_extra_widget( size() );
}

QWidget *ChoqokTabBar::extraWidget( ExtraWidgetPosition position )
{
    return p->extra_wgt_hash.value( position );
}

void ChoqokTabBar::setTabAlongsideWidget( QWidget *widget )
{
    p->tab_alongside_widget = widget;
    init_alongside_widget( size() );
}

QWidget *ChoqokTabBar::tabAlongsideWidget() const
{
    return p->tab_alongside_widget;
}

void ChoqokTabBar::setTabCloseActivatePrevious( bool stt )
{ 
    if( stt )
        setSelectionBehaviorOnRemove( ChoqokTabBar::SelectPreviousTab );
    else
        setSelectionBehaviorOnRemove( ChoqokTabBar::SelectLeftTab );
}

bool ChoqokTabBar::tabCloseActivatePrevious() const
{
    return ( p->selection_behavior == ChoqokTabBar::SelectPreviousTab );
}

ChoqokTabBar::SelectionBehavior ChoqokTabBar::selectionBehaviorOnRemove() const
{
    return p->selection_behavior;
}

void ChoqokTabBar::setSelectionBehaviorOnRemove ( ChoqokTabBar::SelectionBehavior behavior )
{
    if( p->selection_behavior == behavior )
        return;
    
    p->selection_behavior = behavior;
    
    /*! ----------- Setting Up All Linked ChoqokTabBars ------------ */
    if( linkedTabBar() )
        for( int i=0 ; i<choqok_tabbars_list.count() ; i++ )
            choqok_tabbars_list.at(i)->setSelectionBehaviorOnRemove( behavior );
    /*! ------------------------------------------------------------ */
}

QWidget *ChoqokTabBar::currentWidget() const
{
    return p->st_widget->currentWidget();
}

QWidget *ChoqokTabBar::widget( int index ) const
{
    return p->st_widget->widget( index );
}

int ChoqokTabBar::currentIndex() const
{
    return p->st_widget->currentIndex();
}

int ChoqokTabBar::indexOf( QWidget *widget ) const
{
    return p->st_widget->indexOf( widget );
}

int ChoqokTabBar::addTab( QWidget *widget , const QString & name )
{
    return insertTab( count() , widget , QIcon() , name );
}

int ChoqokTabBar::addTab( QWidget *widget , const QIcon & icon , const QString & name )
{
    return insertTab( count() , widget , icon , name );
}

int ChoqokTabBar::insertTab( int index , QWidget *widget , const QString & name )
{
    return insertTab( index , widget , QIcon() , name );
}

int ChoqokTabBar::insertTab( int index , QWidget *widget , const QIcon & input_icon , const QString & name )
{
    QIcon icon( input_icon );
    if( input_icon.isNull() )
        icon = QIcon::fromTheme( "edit-find" );
    
    QAction *action = new QAction( icon , name , this );
        action->setCheckable( true );

    p->actions_list.insert( index , action );
    p->st_widget->insertWidget( index , widget );
    
    connect( widget , SIGNAL(destroyed(QObject*)) , SLOT(widget_destroyed(QObject*)) );

    for( int i=0 ; i<p->history_list.count() ; i++ )
        if( p->history_list.at(i) >= index )
            p->history_list[ i ]++;

    refreshTabBar();
    
    if( count() == 1 )
    {
        action->trigger();
	p->history_list << 0;
    }

    return index;
}

void ChoqokTabBar::moveTab( int from , int to )
{
    int low , high;

    if( from == to ) return ;
    if( from >  to ) { low = to;    high = from; }
    if( from <  to ) { low = from;  high = to;   }


    p->actions_list.move( from , to );
    p->st_widget->move( from , to );


    int shift = (from > to)*2 -1;
    for( int i=0; i<p->history_list.count() ; i++ )
    {
        int index = p->history_list.at(i);
        if( index > low && index < high  )
            p->history_list[ i ] += shift;

        if( index == from )
            p->history_list[ i ] = to;
    }

    refreshTabBar();
    Q_EMIT tabMoved( from , to );
}

void ChoqokTabBar::removeTab( int index )
{
    disconnect( p->st_widget->widget(index) , SIGNAL(destroyed(QObject*)) , this , SLOT(widget_destroyed(QObject*)) );
    
    p->history_list.removeAll( index );
    p->actions_list.removeAt( index );
    p->st_widget->removeWidget( p->st_widget->widget(index) );

    for( int i=0 ; i<p->history_list.count() ; i++ )
        if( p->history_list.at(i) > index )
            p->history_list[ i ]--;

    if( !p->history_list.isEmpty() )
        p->actions_list[ p->history_list.takeFirst() ]->trigger();

    refreshTabBar();
}

void ChoqokTabBar::removePage( QWidget *widget )
{
    removeTab( p->st_widget->indexOf(widget) );
}

void ChoqokTabBar::setTabIcon( int index , const QIcon & input_icon )
{
    p->actions_list[ index ]->setIcon( input_icon );
}

QIcon ChoqokTabBar::tabIcon( int index ) const
{
    return p->actions_list.at(index)->icon();
}

void ChoqokTabBar::setTabText( int index , const QString & text )
{
    p->actions_list[ index ]->setText( text );
}

QString ChoqokTabBar::tabText( int index ) const
{
    return p->actions_list.at(index)->text();
}

void ChoqokTabBar::setLinkedTabBar( bool stt )
{
    if( linkedTabBar() == stt )
        return;
    
    if( !choqok_tabbars_list.isEmpty() && stt )
    {
        ChoqokTabBar *tmp = choqok_tabbars_list.first();
        
        setIconSize( tmp->iconSize() );
        setStyledTabBar( tmp->styledTabBar() );
        setTabPosition( tmp->tabPosition() );
        setSelectionBehaviorOnRemove( tmp->selectionBehaviorOnRemove() );
        setTabsClosable( tmp->tabsClosable() );
        setToolButtonStyle( tmp->toolButtonStyle() );
    }
   
    if( stt )
        choqok_tabbars_list << this;
    else 
        choqok_tabbars_list.removeOne( this );
}

bool ChoqokTabBar::linkedTabBar() const
{
    for( int i=0 ; i<choqok_tabbars_list.count() ; i++ )
        if( choqok_tabbars_list.at(i) == this )
	    return true;
	
    return false;
}

void ChoqokTabBar::setTabBarHidden( bool stt )
{
    p->toolbar->setHidden( stt );
}

bool ChoqokTabBar::isTabBarHidden() const
{
    return p->toolbar->isHidden();
}

QSize ChoqokTabBar::iconSize() const
{
    return p->toolbar->iconSize();
}

void ChoqokTabBar::setIconSize( const QSize & size )
{
    if( size == p->toolbar->iconSize() )
        return;
  
    p->toolbar->setIconSize( size );
    
    /*! ----------- Setting Up All Linked ChoqokTabBars ------------ */
    if( linkedTabBar() )
        for( int i=0 ; i<choqok_tabbars_list.count() ; i++ )
            choqok_tabbars_list.at(i)->setIconSize( size );
    /*! ------------------------------------------------------------ */
    
    Q_EMIT iconSizeChanged( size );
}

int ChoqokTabBar::count() const
{
    return p->st_widget->count();
}

Qt::ToolButtonStyle ChoqokTabBar::toolButtonStyle() const
{
    return p->toolbar->toolButtonStyle();
}

void ChoqokTabBar::setToolButtonStyle( Qt::ToolButtonStyle toolButtonStyle )
{
    if( p->toolbar->toolButtonStyle() == toolButtonStyle )
        return;
  
    p->toolbar->setToolButtonStyle( toolButtonStyle );
    
    /*! ----------- Setting Up All Linked ChoqokTabBars ------------ */
    if( linkedTabBar() )
        for( int i=0 ; i<choqok_tabbars_list.count() ; i++ )
            choqok_tabbars_list.at(i)->setToolButtonStyle( toolButtonStyle );
    /*! ------------------------------------------------------------ */
}
    
void ChoqokTabBar::setStyledTabBar( bool stt )
{
    if( p->styled_tabbar == stt )
        return;
  
    p->styled_tabbar = stt;
    init_style();
    
    /*! ----------- Setting Up All Linked ChoqokTabBars ------------ */
    if( linkedTabBar() )
        for( int i=0 ; i<choqok_tabbars_list.count() ; i++ )
            choqok_tabbars_list.at(i)->setStyledTabBar( stt );
    /*! ------------------------------------------------------------ */
    
    Q_EMIT styledPanelSignal( stt );
}

bool ChoqokTabBar::styledTabBar() const
{
    return p->styled_tabbar;
}

void ChoqokTabBar::refreshTabBar()
{
    p->toolbar->clear();
    for( int i=0 ; i<p->actions_list.count() ; i++ )
        p->toolbar->addAction( p->actions_list.at(i) );
}

void ChoqokTabBar::setCurrentIndex ( int index )
{
    p->actions_list[ index ]->trigger();
}

void ChoqokTabBar::setCurrentWidget ( QWidget *widget )
{
    int index = p->st_widget->indexOf( widget );
    setCurrentIndex( index );
}

void ChoqokTabBar::action_triggered( QAction *action )
{
    action->setChecked( true );

    int new_index = p->actions_list.indexOf(action);
    int old_index = currentIndex();

    if( new_index == old_index )    return;
    if( old_index != -1        )    p->actions_list[ old_index ]->setChecked( false );

    p->st_widget->setCurrentIndex( new_index );
    p->history_list.prepend( new_index );

    Q_EMIT currentChanged( new_index );
}

void ChoqokTabBar::init_style()
{
    if( !styledTabBar() )
    {
        p->toolbar->setStyleSheet( QString() );
        return;
    }
    
    
    /*! ----------------- Setup Colors -------------------- */
    
    QColor tmp = palette().color( QPalette::Active , QPalette::WindowText );
    QString highlight_back( "rgba(%1,%2,%3,%4)" );
        highlight_back = highlight_back.arg( QString::number(tmp.red()) , QString::number(tmp.green()) , QString::number(tmp.blue()) , "113" );
    
    tmp = palette().color( QPalette::Active , QPalette::WindowText );
    QString shadow( "rgba(%1,%2,%3,%4)" );
        shadow = shadow.arg( QString::number(tmp.red()) , QString::number(tmp.green()) , QString::number(tmp.blue()) , "173" );
    
    tmp = palette().color( QPalette::Active , QPalette::Highlight );
    tmp.setHsv( tmp.hue() , tmp.saturation() , tmp.value()/2 );
    QString alt_highlight( "rgba(%1,%2,%3,%4)" );
        alt_highlight = alt_highlight.arg( QString::number(tmp.red()) , QString::number(tmp.green()) , QString::number(tmp.blue()) , "255" );
        
    /*! -------------------------------------------------- */
    
    
    
    p->old_palette = palette();
    switch( static_cast<int>(tabPosition()) )
    {
    case ChoqokTabBar::North :
        p->toolbar->setStyleSheet( 
              "QToolBar{ "
                        "background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 " + shadow + ", stop:0.10 " + alt_highlight + ","
                        " stop:0.90 palette(highlight), stop:1 " + shadow + ");"
                        "border-style: solid;"
                        "padding: 0px}"
              
              "QToolButton{ border-style:solid; background-color: transparent;"
                        "padding-left:   2px;"
                        "padding-right:  2px;"
                        "padding-top:    6px;"
                        "padding-bottom: 6px;"
                        "margin: 0px; }"
              "QToolButton:checked{ background-color: qconicalgradient(cx:0.5, cy:0.85, angle:90, stop:0 transparent, stop:0.3500 " + highlight_back +
                        ", stop:0.3700 palette(window), stop:0.6500 palette(window), stop:0.6700 " + highlight_back + ", stop:1 transparent); }" 
              "QToolButton:hover:!checked{ background-color: qlineargradient(x1:0, y1:3, x2:0, y2:0, stop:0 " + shadow + ",stop:1 transparent); }" 
                                 );
        break;
    case ChoqokTabBar::South :
        p->toolbar->setStyleSheet( 
              "QToolBar{ "
                        "background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 " + shadow + ", stop:0.10 palette(highlight),"
                        " stop:0.90 " + alt_highlight + ", stop:1 " + shadow + ");"
                        "border-style: solid;"
                        "padding: 0px}"
              
              "QToolButton{ border-style:solid; background-color: transparent;"
                        "padding-left:   2px;"
                        "padding-right:  2px;"
                        "padding-top:    6px;"
                        "padding-bottom: 6px;"
                        "margin: 0px; }"
              "QToolButton:checked{ background-color: qconicalgradient(cx:0.5, cy:0.15, angle:270, stop:0 transparent, stop:0.3500 " + highlight_back +
                        ", stop:0.3700 palette(window), stop:0.6500 palette(window), stop:0.6700 " + highlight_back + ", stop:1 transparent); }"  
              "QToolButton:hover:!checked{ background-color: qlineargradient(x1:0, y1:-2, x2:0, y2:1, stop:0 " + shadow + ",stop:1 transparent); }" 
                                 );
        break;
    case ChoqokTabBar::West :
        p->toolbar->setStyleSheet( 
              "QToolBar{ "
                        "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 " + alt_highlight + ", stop:0.90 palette(highlight), "
                          "stop:1 " + shadow + ");"
                        "border-style: solid;"
                        "padding: 0px}"
              
              "QToolButton{ border-style:solid; background-color: transparent;"
                        "padding-left:   6px;"
                        "padding-right:  6px;"
                        "padding-top:    2px;"
                        "padding-bottom: 2px;"
                        "margin: 0px; }"
              "QToolButton:checked{ background-color: qconicalgradient(cx:0.85, cy:0.5, angle:180, stop:0 transparent, stop:0.3500 " + highlight_back +
                        ", stop:0.3700 palette(window), stop:0.6500 palette(window), stop:0.6700 " + highlight_back + ", stop:1 transparent); }" 
              "QToolButton:hover:!checked{ background-color: qlineargradient(x1:3, y1:0, x2:0, y2:0, stop:0 " + shadow + ",stop:1 transparent); }" 
                                 );
        break;
    case ChoqokTabBar::East :
        p->toolbar->setStyleSheet( 
              "QToolBar{ "
                        "background-color: qlineargradient(spread:pad, x1:1, y1:0, x2:0, y2:0, stop:0 " + alt_highlight + ", stop:0.90 palette(highlight), "
                          "stop:1 " + shadow + ");"
                        "border-style: solid;"
                        "padding: 0px}"
              
              "QToolButton{ border-style:solid; background-color: transparent;"
                        "padding-left:   6px;"
                        "padding-right:  6px;"
                        "padding-top:    2px;"
                        "padding-bottom: 2px;"
                        "margin: 0px; }"
              "QToolButton:checked{ background-color: qconicalgradient(cx:0.15, cy:0.5, angle:0, stop:0 transparent, stop:0.3500 " + highlight_back +
                        ", stop:0.3700 palette(window), stop:0.6500 palette(window), stop:0.6700 " + highlight_back + ", stop:1 transparent); }" 
              "QToolButton:hover:!checked{ background-color: qlineargradient(x1:-2, y1:0, x2:1, y2:0, stop:0 " + shadow + ",stop:1 transparent); }" 
                                 );
        break;
    }
}

void ChoqokTabBar::init_extra_widget( const QSize & size )
{
    QWidget *widget;
    
    if( p->corners_hash.contains( Qt::TopLeftCorner ) )
    {
        widget = p->corners_hash.value( Qt::TopLeftCorner );
        widget->move( 0 , 0 );
    }
    
    if( p->corners_hash.contains( Qt::TopRightCorner ) )
    {
        widget = p->corners_hash.value( Qt::TopRightCorner );
        widget->move( size.width() - widget->width() , 0 );
    }
    
    if( p->corners_hash.contains( Qt::BottomLeftCorner ) )
    {
        widget = p->corners_hash.value( Qt::BottomLeftCorner );
        widget->move( 0 , size.height() - widget->height() );
    }
    
    if( p->corners_hash.contains( Qt::BottomRightCorner ) )
    {
        widget = p->corners_hash.value( Qt::BottomRightCorner );
        widget->move( size.width() - widget->width() , size.height() - widget->height() );
    }
}

void ChoqokTabBar::init_alongside_widget( const QSize & size )
{
    if( !p->tab_alongside_widget )
        return;
    
    QWidget *widget = p->tab_alongside_widget;
    switch( static_cast<int>(tabPosition()) )
    {
    case North :
        widget->move( size.width() - widget->width() , 0 );
        break;
        
    case South :
        widget->move( size.width() - widget->width() , size.height() - widget->height() );
        break;
        
    case West :
        widget->move( 0 , size.height() - widget->height() );
        break;
        
    case East :
        widget->move( size.width() - widget->width() , size.height() - widget->height() );
        break;
    }
}

void ChoqokTabBar::resizeEvent( QResizeEvent *event )
{
    QWidget::resizeEvent( event );
    init_extra_widget( event->size() );
    init_alongside_widget( event->size() );
}

void ChoqokTabBar::paintEvent( QPaintEvent * )
{
    if( p->old_palette != palette() )
        init_style();
}

void ChoqokTabBar::contextMenuRequest( const QPoint & )
{
    const QPoint & global_point = QCursor::pos();
    const QPoint & local_point  = mapFromGlobal( global_point );
    
    QAction *action = p->toolbar->actionAt( local_point );
    if( action )
    {
        Q_EMIT contextMenu( global_point );
        Q_EMIT contextMenu( widget(p->actions_list.indexOf(action)) , global_point );
        return;
    }
    
    KAction north( i18n("Top") , this );
    KAction west(  i18n("Left")  , this );
    KAction east(  i18n("Right")  , this );
    KAction south( i18n("Bottom") , this );
    KAction size_s( i18n("Small")  , this );
    KAction size_m( i18n("Medium") , this );
    KAction size_b( i18n("Big")    , this );
    KAction styled( i18n("Styled Panel") , this );
    
    /*! ------------- Setting Up Datas --------------- */
    north.setData( ChoqokTabBar::North );
    west.setData(  ChoqokTabBar::West  );
    east.setData(  ChoqokTabBar::East  );
    south.setData( ChoqokTabBar::South );
    
    size_s.setData( ICON_SMALL_SIZE  );
    size_m.setData( ICON_MEDIUM_SIZE );
    size_b.setData( ICON_BIG_SIZE    );
    /*! ------------------------------------------------ */
    
    
    /*! ------------- Setting Up Actions --------------- */
    north.setCheckable(  true );
    west.setCheckable(   true );
    east.setCheckable(   true );
    south.setCheckable(  true );
    size_s.setCheckable( true );
    size_m.setCheckable( true );
    size_b.setCheckable( true );
    styled.setCheckable( true );
    /*! ------------------------------------------------ */
    
    
    
    /*! ------------- Setting Up Checks --------------- */
    switch( static_cast<int>(tabPosition()) )
    {
    case ChoqokTabBar::North :
        north.setChecked( true );
        break;
    case ChoqokTabBar::South :
        south.setChecked( true );
        break;
    case ChoqokTabBar::West :
        west.setChecked( true );
        break;
    case ChoqokTabBar::East :
        east.setChecked( true );
        break;
    }
    
    if( iconSize() == QSize(ICON_SMALL_SIZE,ICON_SMALL_SIZE) )
        size_s.setChecked( true );
    else if( iconSize() == QSize(ICON_MEDIUM_SIZE,ICON_MEDIUM_SIZE) )
        size_m.setChecked( true );
    else if( iconSize() == QSize(ICON_BIG_SIZE,ICON_BIG_SIZE) )
        size_b.setChecked( true );
    
    styled.setChecked( styledTabBar() );
    /*! ------------------------------------------------ */
 
    
    KMenu menu;
    menu.addAction( &north   );
    menu.addAction( &west    );
    menu.addAction( &east    );
    //menu.addAction( &south   );
    menu.addSeparator();
    menu.addAction( &size_s );
    menu.addAction( &size_m );
    menu.addAction( &size_b );
    menu.addSeparator();
    menu.addAction( &styled  );
        

    
    QAction *result = menu.exec( global_point );
    if( !result )
        return;
    
    else if( result == &styled )
        setStyledTabBar( result->isChecked() );
    
    else if( result == &size_s || result == &size_m || result == &size_b  )
        setIconSize( QSize( result->data().toInt() , result->data().toInt() ) );
    else
        setTabPosition( static_cast<ChoqokTabBar::TabPosition>(result->data().toInt()) );
}

void ChoqokTabBar::widget_destroyed( QObject *obj )
{
    removePage( static_cast<QWidget*>(obj) );
}

ChoqokTabBar::~ChoqokTabBar()
{
    Choqok::AppearanceSettings::setTabBarPosition(tabPosition());
    Choqok::AppearanceSettings::setTabBarSize(iconSize().width());
    Choqok::AppearanceSettings::setTabBarIsStyled(p->styled_tabbar);
    Choqok::AppearanceSettings::self()->writeConfig();
    setLinkedTabBar( false );
    delete p;
}

}
}
