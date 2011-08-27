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

#include "mainwindow.h"
#include "accountmanager.h"
// #include "searchwindow.h"
#include "systrayicon.h"
#include "quickpost.h"

#include <KTabWidget>
#include <kconfigdialog.h>
#include <kstatusbar.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <KDE/KLocale>
#include <KMessageBox>
#include <QTimer>
#include <postwidget.h>
#include <microblogwidget.h>
#include <pluginmanager.h>
#include <passwordmanager.h>
#include <mediamanager.h>
#include <QWheelEvent>
#include <QMenu>
#include <KXMLGUIFactory>
#include <choqokuiglobal.h>
#include <choqokappearancesettings.h>
#include "choqokapplication.h"
#include <ksettings/dialog.h>
#include <choqokbehaviorsettings.h>
#include <kstandarddirs.h>
#include <KSplashScreen>
#include <KMenu>
#include "uploadmediadialog.h"
#include <knotifyconfigwidget.h>
#include <KMenuBar>
#include <KPushButton>

const char* mainButtonStyleSheet = "QPushButton{\
background-color: qlineargradient(spread:reflect, x1:0.449382, y1:0, x2:0.448, y2:1, stop:0.15 rgba(255, 255, 255, 100), stop:1 rgba(61, 158, 0, 255));\
    border: none;\
    border-radius: 4px;\
    width: 70px;\
    height: 20px;\
    }\
    QPushButton:hover{\
        border: 2px solid rgba(170,170,255,180);\
    }\
    QPushButton:pressed{\
    background-color: qlineargradient(spread:reflect, x1:0.449382, y1:0, x2:0.448, y2:1, stop:0.3 rgba(255, 255, 255, 100), stop:1 rgba(61, 158, 0, 255));\
    }";

MainWindow::MainWindow()
    : Choqok::UI::MainWindow(), quickWidget(0), s_settingsDialog(0), m_splash(0),
    choqokMainButton(0), microblogCounter(0)
{
    kDebug();
    setAttribute ( Qt::WA_DeleteOnClose, false );
    setAttribute ( Qt::WA_QuitOnClose, false );

    timelineTimer = new QTimer( this );
    setWindowTitle( i18n("Choqok") );
    connect( mainWidget, SIGNAL(currentChanged(int)), SLOT(slotCurrentBlogChanged(int)) );
    setCentralWidget( mainWidget );

    sysIcon = new SysTrayIcon(this);
//     sysIcon->show();
    setupActions();
    statusBar()->show();
    setupGUI();

    if ( Choqok::BehaviorSettings::updateInterval() > 0 )
        mPrevUpdateInterval = Choqok::BehaviorSettings::updateInterval();
    else
        mPrevUpdateInterval = 10;

    connect( timelineTimer, SIGNAL( timeout() ), this, SIGNAL( updateTimelines() ) );
    connect( this, SIGNAL(markAllAsRead()), SLOT(slotMarkAllAsRead()) );
    connect( Choqok::AccountManager::self(), SIGNAL( accountAdded(Choqok::Account*)),
             this, SLOT( addBlog(Choqok::Account*)));
    connect( Choqok::AccountManager::self(), SIGNAL( accountRemoved( const QString& ) ),
             this, SLOT( removeBlog(QString)) );
    connect( Choqok::AccountManager::self(), SIGNAL(allAccountsLoaded()),
             SLOT(loadAllAccounts()) );

    connect( Choqok::PluginManager::self(), SIGNAL(pluginLoaded(Choqok::Plugin*)),
             this, SLOT(newPluginAvailable(Choqok::Plugin*)) );

    QTimer::singleShot(0, Choqok::PluginManager::self(), SLOT( loadAllPlugins() ) );
//     Choqok::AccountManager::self()->loadAllAccounts();
    QTimer::singleShot(0, Choqok::AccountManager::self(), SLOT( loadAllAccounts() ) );

    connect(this, SIGNAL(updateTimelines()), SLOT(slotUpdateTimelines()));

    QPoint pos = Choqok::BehaviorSettings::position();
    if(pos.x() != -1 && pos.y() != -1) {
        move(pos);
    }
    actionCollection()->action("choqok_hide_menubar")->setChecked(menuBar()->isHidden());
}

MainWindow::~MainWindow()
{
    kDebug();
}

void MainWindow::loadAllAccounts()
{
    kDebug();

    if( Choqok::BehaviorSettings::showSplashScreen() ){
        KStandardDirs *stdDirs = KGlobal::dirs();
        QString img = stdDirs->findResource( "data", "choqok/images/splash_screen.png" );
        //         kDebug()<<img;
        QPixmap splashpix( img );
        if(splashpix.isNull())
            kDebug()<<"Pixmap is NULL";
        m_splash = new KSplashScreen( splashpix, Qt::WindowStaysOnTopHint );
        m_splash->show();
    }

    settingsChanged();
    QList<Choqok::Account*> accList = Choqok::AccountManager::self()->accounts();
    int count = microblogCounter = accList.count();
    if( count > 0 ) {
        for( int i=0; i < count; ++i ){
            addBlog(accList.at(i), true);
        }
        kDebug()<<"All accounts loaded.";
        if(Choqok::BehaviorSettings::updateInterval() > 0)
            QTimer::singleShot(500, this, SIGNAL(updateTimelines()));
    } else {
        delete m_splash;
        m_splash = 0;
    }
    ChoqokApplication::setStartingUp(false);
    createQuickPostDialog();
}

void MainWindow::newPluginAvailable( Choqok::Plugin *plugin )
{
    kDebug();
    guiFactory()->addClient(plugin);
}

void MainWindow::nextTab(int delta,Qt::Orientation orientation)
{
  if(!isVisible())
    return;
  KTabWidget * widget = 0;
  switch(orientation) {
  case Qt::Vertical:
    widget = mainWidget;
  break;
  case Qt::Horizontal:
      ///Commented for now!
//     Choqok::MicroBlogWidget * t = qobject_cast<Choqok::MicroBlogWidget*>( mainWidget->widget( mainWidget->currentIndex() ));
//     if(t)
//       widget = t->tabs;
//     else
      return;
  break;
  }
  if(!widget) return;

  int count = widget->count();
  int index = widget->currentIndex();
  int page;
  if(delta > 0) {
    page = index>0?index-1:count-1;
  } else {
    page = index<count-1?index+1:0;
  }
  widget->setCurrentIndex(page);
}

void MainWindow::setupActions()
{
    actQuit = KStandardAction::quit( this, SLOT( slotQuit() ), actionCollection() );
    prefs = KStandardAction::preferences( this, SLOT( slotConfigChoqok() ), actionCollection() );

    KAction *actUpdate = new KAction( KIcon( "view-refresh" ), i18n( "Update Timelines" ), this );
    actionCollection()->addAction( QLatin1String( "update_timeline" ), actUpdate );
    actUpdate->setShortcut( Qt::Key_F5 );
    KShortcut updateGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_F5 );
//     updateGlobalShortcut.setAlternate ( Qt::MetaModifier | Qt::Key_F5 );
    actUpdate->setGlobalShortcut( updateGlobalShortcut );
    connect( actUpdate, SIGNAL( triggered( bool ) ), this, SIGNAL( updateTimelines() ) );
//     connect( actUpdate, SIGNAL( triggered( bool ) ), this, SIGNAL( updateSearchResults() ) );

    KAction *newTwit = new KAction( KIcon( "document-new" ), i18n( "Quick Post" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_new_post" ), newTwit );
    newTwit->setShortcut( KShortcut( Qt::CTRL | Qt::Key_T ) );
    KShortcut quickTwitGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_T );
    newTwit->setGlobalShortcut( quickTwitGlobalShortcut );
    connect( newTwit, SIGNAL( triggered(bool) ), this, SLOT( triggerQuickPost()) );

    KAction *markRead = new KAction( KIcon( "mail-mark-read" ), i18n( "Mark All As Read" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_mark_as_read" ), markRead );
    markRead->setShortcut( KShortcut( Qt::CTRL | Qt::Key_R ) );
    connect( markRead, SIGNAL( triggered( bool ) ), this, SIGNAL( markAllAsRead()) );

    showMain = new KAction( this );
    actionCollection()->addAction( QLatin1String( "toggle_mainwin" ), showMain );
    KShortcut toggleMainGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_C );
    showMain->setGlobalShortcut( toggleMainGlobalShortcut/*, KAction::DefaultShortcut, KAction::NoAutoloading*/ );
    if(this->isVisible())
        showMain->setText( i18n( "Minimize" ) );
    else
        showMain->setText( i18n("Restore") );
    connect( showMain, SIGNAL( triggered( bool ) ), this, SLOT( toggleMainWindow() ) );

    KAction *act = KStandardAction::configureNotifications ( this, SLOT ( slotConfNotifications() ),
                                                             actionCollection() );
    actionCollection()->addAction ( "settings_notifications", act );

    KAction *enableUpdates = new KAction( i18n( "Enable Update Timer" ), this );
    enableUpdates->setCheckable( true );
    actionCollection()->addAction( QLatin1String( "choqok_enable_updates" ), enableUpdates );
    enableUpdates->setShortcut( KShortcut( Qt::CTRL | Qt::Key_U ) );
    connect( enableUpdates, SIGNAL( toggled( bool ) ), this, SLOT( setTimeLineUpdatesEnabled( bool ) ) );

    KAction *enableNotify = new KAction( i18n( "Enable Notifications" ), this );
    enableNotify->setCheckable( true );
    actionCollection()->addAction( QLatin1String( "choqok_enable_notify" ), enableNotify );
    enableNotify->setShortcut( KShortcut( Qt::CTRL | Qt::Key_N ) );
    connect( enableNotify, SIGNAL( toggled( bool ) ), this, SLOT( setNotificationsEnabled( bool ) ) );

    KAction* hideMenuBar = new KAction( i18n("Hide Menubar"), this );
    hideMenuBar->setCheckable(true);
    actionCollection()->addAction( QLatin1String( "choqok_hide_menubar" ), hideMenuBar );
    hideMenuBar->setShortcut( KShortcut(Qt::ControlModifier | Qt::Key_M) );
    connect( hideMenuBar, SIGNAL(toggled(bool)), menuBar(), SLOT(setHidden(bool)) );
    connect( hideMenuBar, SIGNAL(toggled(bool)), this, SLOT(slotShowSpecialMenu(bool)) );

    KAction *clearAvatarCache = new KAction(KIcon("edit-clear"), i18n( "Clear Avatar Cache" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_clear_avatar_cache" ), clearAvatarCache );
    QString tip = i18n( "You have to restart Choqok to load avatars again" );
    clearAvatarCache->setToolTip(tip);
    clearAvatarCache->setStatusTip(tip);
    connect( clearAvatarCache, SIGNAL( triggered() ),
             Choqok::MediaManager::self(), SLOT(clearImageCache()) );

    KAction *uploadMedium = new KAction( KIcon("arrow-up"), i18n( "Upload Medium..." ), this );
    actionCollection()->addAction( QLatin1String( "choqok_upload_medium" ), uploadMedium );
    connect( uploadMedium, SIGNAL( triggered(bool)), this, SLOT(slotUploadMedium()) );

    ///SysTray Actions:
    sysIcon->contextMenu()->addAction( newTwit );
//     sysIcon->contextMenu()->addAction( uploadMedium );
    sysIcon->contextMenu()->addAction( actUpdate );
    sysIcon->contextMenu()->addSeparator();
    connect( enableUpdates, SIGNAL( toggled( bool ) ), sysIcon, SLOT( setTimeLineUpdatesEnabled( bool ) ) );
    sysIcon->contextMenu()->addAction( enableUpdates );
    sysIcon->setTimeLineUpdatesEnabled( enableUpdates->isChecked() );
//     sysIcon->contextMenu()->addAction( enableNotify );
    sysIcon->contextMenu()->addAction( prefs );

    sysIcon->contextMenu()->addSeparator();
    sysIcon->contextMenu()->addAction(showMain);
    sysIcon->contextMenu()->addAction(actQuit);
//     connect( sysIcon, SIGNAL(quitSelected()), this, SLOT(slotQuit()) );
    connect(sysIcon, SIGNAL(scrollRequested(int,Qt::Orientation)),
            this, SLOT(nextTab(int,Qt::Orientation)));
}

void MainWindow::slotConfNotifications()
{
    KNotifyConfigWidget::configure ( this );
}

void MainWindow::createQuickPostDialog()
{
    quickWidget = new Choqok::UI::QuickPost( this );
    Choqok::UI::Global::setQuickPostWidget(quickWidget);
    quickWidget->setAttribute(Qt::WA_DeleteOnClose, false);
    connect( quickWidget, SIGNAL( newPostSubmitted(Choqok::JobResult)),
             sysIcon, SLOT( slotJobDone(Choqok::JobResult)) );
    emit(quickPostCreated());
}

void MainWindow::triggerQuickPost()
{
    if ( Choqok::AccountManager::self()->accounts().isEmpty() )
    {
        KMessageBox::error( this, i18n ( "No account created. You have to create an account before being able to make posts." ) );
        return;
    }
    if(!quickWidget)
        createQuickPostDialog();
    if ( quickWidget->isVisible() ) {
        quickWidget->hide();
    } else {
        quickWidget->show();
    }
}

void MainWindow::slotConfigChoqok()
{
    if ( !s_settingsDialog )
    {
        s_settingsDialog = new KSettings::Dialog( this );
    }
    s_settingsDialog->show();
    connect(Choqok::BehaviorSettings::self(), SIGNAL(configChanged()),
            SLOT(slotBehaviorConfigChanged()) );
    connect(Choqok::AppearanceSettings::self(), SIGNAL(configChanged()),
            SLOT(slotAppearanceConfigChanged()) );
}

void MainWindow::settingsChanged()
{
    kDebug();
    if ( Choqok::AccountManager::self()->accounts().count() < 1 ) {
        if ( KMessageBox::questionYesNo( this, i18n( "In order to use Choqok you need \
an account at one of the supported micro-blogging services.\n\
Would you like to add your account now?" ) ) == KMessageBox::Yes ) {
            slotConfigChoqok();
        }
    }
    slotAppearanceConfigChanged();
    slotBehaviorConfigChanged();
}

void MainWindow::slotAppearanceConfigChanged()
{
    if ( Choqok::AppearanceSettings::isCustomUi() ) {
        Choqok::UI::PostWidget::setStyle( Choqok::AppearanceSettings::unreadForeColor() ,
                                          Choqok::AppearanceSettings::unreadBackColor(),
                                          Choqok::AppearanceSettings::readForeColor() ,
                                          Choqok::AppearanceSettings::readBackColor() ,
                                         Choqok::AppearanceSettings::ownForeColor() ,
                                         Choqok::AppearanceSettings::ownBackColor(),
                                         Choqok::AppearanceSettings::font() );
    } else {
        QPalette p = window()->palette();
        Choqok::UI::PostWidget::setStyle( p.color(QPalette::WindowText) , p.color(QPalette::Window).lighter() ,
                                          p.color(QPalette::WindowText) , p.color(QPalette::Window) ,
                                          p.color(QPalette::WindowText) , p.color(QPalette::Window),
                                          font() );
    }
    int count = mainWidget->count();
    for ( int i = 0; i < count; ++i ) {
        qobject_cast<Choqok::UI::MicroBlogWidget *>( mainWidget->widget( i ) )->settingsChanged();
    }
}

void MainWindow::slotBehaviorConfigChanged()
{
    if ( Choqok::BehaviorSettings::notifyEnabled() ) {
        actionCollection()->action( "choqok_enable_notify" )->setChecked( true );
    } else {
        actionCollection()->action( "choqok_enable_notify" )->setChecked( false );
    }
    if ( Choqok::BehaviorSettings::updateInterval() > 0 ) {
        timelineTimer->setInterval( Choqok::BehaviorSettings::updateInterval() *60000 );
        timelineTimer->start();
        actionCollection()->action( "choqok_enable_updates" )->setChecked( true );
    } else {
        timelineTimer->stop();
        actionCollection()->action( "choqok_enable_updates" )->setChecked( false );
    }
}

void MainWindow::slotQuit()
{
    kDebug();
    ChoqokApplication *app = qobject_cast<ChoqokApplication*>(kapp);
    app->quitChoqok();
}

bool MainWindow::queryClose()
{
    return true;
}

bool MainWindow::queryExit()
{
    kDebug();
    ChoqokApplication *app = qobject_cast<ChoqokApplication*>(kapp);
    if( app->sessionSaving() || app->isShuttingDown() ) {
        Choqok::BehaviorSettings::setPosition( pos() );
        timelineTimer->stop();
        Choqok::BehaviorSettings::self()->writeConfig();
        kDebug () << " shutting down plugin manager";
        Choqok::PluginManager::self()->shutdown();
//         Choqok::PasswordManager::self()->deleteLater();
//         Choqok::MediaManager::self()->deleteLater();
//         return true;
    }/* else
        return false;*/
    return true;
}

void MainWindow::disableApp()
{
    kDebug();
    timelineTimer->stop();
//     kDebug()<<"timelineTimer stoped";
    actionCollection()->action( "update_timeline" )->setEnabled( false );
    actionCollection()->action( "choqok_new_post" )->setEnabled( false );
//     actionCollection()->action( "choqok_search" )->setEnabled( false );
    actionCollection()->action( "choqok_mark_as_read" )->setEnabled( false );
//     actionCollection()->action( "choqok_now_listening" )->setEnabled( false );
}

void MainWindow::enableApp()
{
    kDebug();
    if ( Choqok::BehaviorSettings::updateInterval() > 0 ) {
        timelineTimer->start();
//         kDebug()<<"timelineTimer started";
    }
    actionCollection()->action( "update_timeline" )->setEnabled( true );
    actionCollection()->action( "choqok_new_post" )->setEnabled( true );
//     actionCollection()->action( "choqok_search" )->setEnabled( true );
    actionCollection()->action( "choqok_mark_as_read" )->setEnabled( true );
//     actionCollection()->action( "choqok_now_listening" )->setEnabled( true );
}

void MainWindow::addBlog( Choqok::Account * account, bool isStartup )
{
    kDebug() << "Adding new Blog, Alias: " << account->alias() << "Blog: " << account->microblog()->serviceName();

    Choqok::UI::MicroBlogWidget *widget = account->microblog()->createMicroBlogWidget(account, this);
    connect(widget, SIGNAL(loaded()), SLOT(oneMicroblogLoaded()));
    connect( widget, SIGNAL(updateUnreadCount(int,int)), SLOT(slotUpdateUnreadCount(int,int)) );
    widget->initUi();

//     connect( widget, SIGNAL( sigSetUnread( int ) ), sysIcon, SLOT( slotSetUnread( int ) ) );
    /*connect( widget, SIGNAL( showStatusMessage(QString,bool)),
             this, SLOT( showStatusMessage( const QString&, bool ) ) )*/;
    connect( widget, SIGNAL( showMe() ), this, SLOT( showBlog()) );

    connect( this, SIGNAL( updateTimelines() ), widget, SLOT( updateTimelines() ) );
    connect( this, SIGNAL( markAllAsRead() ), widget, SLOT( markAllAsRead() ) );
    connect( this, SIGNAL(removeOldPosts()), widget, SLOT(removeOldPosts()) );
//     kDebug()<<"Plugin Icon: "<<account->microblog()->pluginIcon();
    mainWidget->addTab( widget, KIcon(account->microblog()->pluginIcon()), account->alias() );

    if( !isStartup )
        QTimer::singleShot( 1500, widget, SLOT( updateTimelines() ) );
    enableApp();
    if( mainWidget->count() > 1)
        mainWidget->setTabBarHidden(false);
    else
        mainWidget->setTabBarHidden(true);
}

void MainWindow::removeBlog( const QString & alias )
{
    kDebug();
    int count = mainWidget->count();
    for ( int i = 0; i < count; ++i ) {
        Choqok::UI::MicroBlogWidget * tmp = qobject_cast<Choqok::UI::MicroBlogWidget *>( mainWidget->widget( i ) );
        if ( tmp->currentAccount()->alias() == alias ) {
            mainWidget->removeTab( i );
        if ( mainWidget->count() < 1 )
            disableApp();
        tmp->deleteLater();
        if( mainWidget->count() > 1)
            mainWidget->setTabBarHidden(false);
        else
            mainWidget->setTabBarHidden(true);
        return;
        }
    }
}

void MainWindow::slotUpdateUnreadCount(int change, int sum)
{
    kDebug()<<"Change: "<<change<<" Sum: "<<sum;
    Choqok::UI::MicroBlogWidget *wd = qobject_cast<Choqok::UI::MicroBlogWidget*>(sender());
    sysIcon->updateUnreadCount(change);
    if( sysIcon->unreadCount() )
        setWindowTitle( i18n("Choqok (%1)", sysIcon->unreadCount()) );
    else
        setWindowTitle( i18n("Choqok") );
    if(wd) {
        int tabIndex = mainWidget->indexOf(wd);
        if(tabIndex == -1)
            return;
        if(sum > 0)
            mainWidget->setTabText( tabIndex, wd->currentAccount()->alias() + QString("(%1)").arg(sum) );
        else
            mainWidget->setTabText( tabIndex, wd->currentAccount()->alias() );
    }
}

void MainWindow::showBlog()
{
    mainWidget->setCurrentWidget( qobject_cast<QWidget*>( sender() ) );
    if ( !this->isVisible() )
        this->show();
    this->raise();
}

void MainWindow::setTimeLineUpdatesEnabled( bool isEnabled )
{
    kDebug();
    if ( isEnabled ) {
        if( mPrevUpdateInterval > 0 )
            Choqok::BehaviorSettings::setUpdateInterval( mPrevUpdateInterval );
        emit updateTimelines();
        timelineTimer->start( Choqok::BehaviorSettings::updateInterval() *60000 );
//         kDebug()<<"timelineTimer started";
    } else {
        mPrevUpdateInterval = Choqok::BehaviorSettings::updateInterval();
        timelineTimer->stop();
//         kDebug()<<"timelineTimer stoped";
        Choqok::BehaviorSettings::setUpdateInterval( 0 );
    }
}

void MainWindow::setNotificationsEnabled( bool isEnabled )
{
    kDebug();
    if ( isEnabled ) {
        Choqok::BehaviorSettings::setNotifyEnabled( true );
    } else {
        Choqok::BehaviorSettings::setNotifyEnabled( false );
    }
}

void MainWindow::toggleMainWindow()
{
    if( this->isVisible() )
        hide();
    else
        show();
}

void MainWindow::hideEvent(QHideEvent* event)
{
    Choqok::UI::MainWindow::hideEvent(event);
    showMain->setText( i18n("Restore") );
}

void MainWindow::showEvent(QShowEvent* event)
{
    Choqok::UI::MainWindow::showEvent(event);
    showMain->setText( i18n("Minimize") );
}

void MainWindow::slotMarkAllAsRead()
{
    setWindowTitle( i18n("Choqok") );
    sysIcon->resetUnreadCount();
    int count = mainWidget->count();
    for(int i=0; i<count; ++i) {
        Choqok::UI::MicroBlogWidget *wd = qobject_cast<Choqok::UI::MicroBlogWidget*>(mainWidget->widget(i));
        mainWidget->setTabText( i, wd->currentAccount()->alias() );
    }
}

void MainWindow::slotCurrentBlogChanged(int)
{
    Choqok::UI::MicroBlogWidget *wd = qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->currentWidget());
    if( wd ) {
        wd->setFocus();
        emit currentMicroBlogWidgetChanged(wd);
    }
}

void MainWindow::oneMicroblogLoaded()
{
    kDebug();
    --microblogCounter;
    if(microblogCounter < 1){
        //Everything loaded, So splash screen should be deleted!
        delete m_splash;
        m_splash = 0;
        if( Choqok::BehaviorSettings::showMainWinOnStart() )
            this->show();
    }
}

void MainWindow::slotUpdateTimelines()
{
    if ( Choqok::AccountManager::self()->accounts().count() )
         showStatusMessage( i18n( "Loading timelines..." ) );
}

void MainWindow::slotUploadMedium()
{
    QPointer<Choqok::UI::UploadMediaDialog> dlg = new Choqok::UI::UploadMediaDialog(this);
    dlg->show();
}

void MainWindow::slotShowSpecialMenu(bool show)
{
    if(show) {
        if(!choqokMainButton) {
            choqokMainButton = new KPushButton(KIcon("choqok"), QString(), mainWidget);
//             choqokMainButton->setFixedWidth(50);
//             choqokMainButton->setStyleSheet(mainButtonStyleSheet);
//             QPalette p = choqokMainButton->palette();
//             p.setColor(QPalette::Button, QColor(/*244, 165, 5*/10, 207, 135));
//             choqokMainButton->setPalette(p);
            KMenu* menu = new KMenu(i18n("Choqok"), choqokMainButton);
            menu->addAction(actionCollection()->action("choqok_new_post"));
            menu->addAction(actionCollection()->action("update_timeline"));
            menu->addAction(actionCollection()->action("choqok_mark_as_read"));
            menu->addAction(actionCollection()->action("choqok_upload_medium"));
            menu->addSeparator();
            menu->addAction(actionCollection()->action("choqok_enable_updates"));
            menu->addAction(actionCollection()->action("choqok_hide_menubar"));
            menu->addAction(prefs);
            menu->addSeparator();
            menu->addAction(showMain);
            menu->addAction(actQuit);
            choqokMainButton->setMenu(menu);
        }
        mainWidget->setCornerWidget(choqokMainButton/*, Qt::TopLeftCorner*/);
        choqokMainButton->show();
    } else {
        mainWidget->setCornerWidget(0/*, Qt::TopLeftCorner*/);
    }
}

#include "mainwindow.moc"
