/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "settings.h"
#include "timelinewidget.h"
#include "constants.h"
#include "accounts.h"
#include "accountmanager.h"
#include "accountswizard.h"
#include "searchwindow.h"
#include "systrayicon.h"
#include "quicktwit.h"
#include "statuswidget.h"
#include "mediamanager.h"

#include <KTabWidget>
#include <kconfigdialog.h>
#include <kstatusbar.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <KDE/KLocale>
#include <KMessageBox>
#include <QProcess>
#include <QTimer>
#include <QMenu>
#include <KNotification>
#include <QDBusInterface>
#include <QDBusReply>
#include "advancedconfig.h"

MainWindow::MainWindow()
    : KXmlGuiWindow()
{
    kDebug();
    quickWidget = 0;
    timelineTimer = new QTimer( this );
    setWindowTitle( i18n("Choqok") );

    mainWidget = new KTabWidget( this );

    setCentralWidget( mainWidget );
    sysIcon = new SysTrayIcon(this);
//     setupQuickTweet();
    setupActions();
    statusBar()->show();
    notify( i18n( "Initializing Choqok, please wait...." ) );
    setupGUI();

//     timelineTimer->setInterval ( Settings::updateInterval() *60000 );
//     timelineTimer->start();
    if ( Settings::notifyType() == SettingsBase::NoNotify )
        mPrevNotifyType = 1;
    else
        mPrevNotifyType = Settings::notifyType();
    if ( Settings::updateInterval() > 0 )
        mPrevUpdateInterval = Settings::updateInterval();
    else
        mPrevUpdateInterval = 10;

    connect( timelineTimer, SIGNAL( timeout() ), this, SIGNAL( updateTimeLines() ) );
    connect( AccountManager::self(), SIGNAL( accountAdded( const Account& ) ),
             this, SLOT( addAccountTimeLine( const Account& ) ) );
    connect( AccountManager::self(), SIGNAL( accountRemoved( const QString& ) ),
             this, SLOT( removeAccountTimeLine( const QString& ) ) );
    settingsChanged();

    QPoint pos = Settings::position();
    if(pos.x() != -1 && pos.y() != -1) {
        move(pos);
    }

    QTimer::singleShot( 0, this, SLOT( loadAccounts() ) );
}

MainWindow::~MainWindow()
{
    kDebug();
}

void MainWindow::nextTab(const QWheelEvent & event) {
  if(!isVisible())
    return;
  KTabWidget * widget = 0;
  switch(event.orientation()) {
  case Qt::Vertical:
    widget = mainWidget;
  break;
  case Qt::Horizontal:
    TimeLineWidget * t = qobject_cast<TimeLineWidget*>(mainWidget->widget( mainWidget->currentIndex() ));
    if(t)
      widget = t->tabs;
    else
      return;
  break;
  }
  if(!widget) return;

  int count = widget->count();
  int index = widget->currentIndex();
  int page;
  if(event.delta() > 0) {
    page = index>0?index-1:count-1;
  } else {
    page = index<count-1?index+1:0;
  }
  widget->setCurrentIndex(page);
}

void MainWindow::setupActions()
{
    KStandardAction::quit( qApp, SLOT( quit() ), actionCollection() );
    connect( qApp, SIGNAL( aboutToQuit() ), this, SLOT( quitApp() ) );
    KAction *prefs = KStandardAction::preferences( this, SLOT( optionsPreferences() ), actionCollection() );

    KAction *actUpdate = new KAction( KIcon( "view-refresh" ), i18n( "Update Timelines" ), this );
    actionCollection()->addAction( QLatin1String( "update_timeline" ), actUpdate );
    actUpdate->setShortcut( Qt::Key_F5 );
    actUpdate->setGlobalShortcutAllowed( true );
    KShortcut updateGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_F5 );
//     updateGlobalShortcut.setAlternate ( Qt::MetaModifier | Qt::Key_F5 );
    actUpdate->setGlobalShortcut( updateGlobalShortcut );
    connect( actUpdate, SIGNAL( triggered( bool ) ), this, SIGNAL( updateTimeLines() ) );
    connect( actUpdate, SIGNAL( triggered( bool ) ), this, SIGNAL( updateSearchResults() ) );

    KAction *newTwit = new KAction( KIcon( "document-new" ), i18n( "Quick Tweet" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_new_twit" ), newTwit );
    newTwit->setShortcut( KShortcut( Qt::CTRL | Qt::Key_T ) );
    newTwit->setGlobalShortcutAllowed( true );
    KShortcut quickTwitGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_T );
    newTwit->setGlobalShortcut( quickTwitGlobalShortcut );
    connect( newTwit, SIGNAL( triggered(bool) ), this, SLOT( postQuickTwit() ) );

    KAction *nowListening = new KAction( KIcon("amarok"), i18n("Post Now Listening"), this);
    actionCollection()->addAction(QLatin1String("choqok_now_listening"), nowListening);
    nowListening->setShortcut(KShortcut( Qt::CTRL | Qt::Key_L ));
    nowListening->setGlobalShortcutAllowed(true);
    nowListening->setGlobalShortcut( KShortcut(Qt::CTRL | Qt::META | Qt::Key_L) );
    connect(nowListening, SIGNAL(triggered(bool)), SLOT(postNowListening()));

    KAction *newSearch = new KAction( KIcon( "edit-find" ), i18n( "Search" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_search" ), newSearch );
    newSearch->setShortcut( KShortcut( Qt::CTRL | Qt::Key_F ) );
    connect( newSearch, SIGNAL( triggered( bool ) ), this, SLOT( search() ) );

    KAction *markRead = new KAction( KIcon( "mail-mark-read" ), i18n( "Mark All As Read" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_mark_read" ), markRead );
    markRead->setShortcut( KShortcut( Qt::CTRL | Qt::Key_R ) );
    connect( markRead, SIGNAL( triggered( bool ) ), this, SIGNAL( setUnreadStatusesToReadState() ) );

    KAction *showMain = new KAction( this );
    actionCollection()->addAction( QLatin1String( "toggle_mainwin" ), showMain );
    KShortcut toggleMainGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_C );
    showMain->setGlobalShortcutAllowed( true );
    showMain->setGlobalShortcut( toggleMainGlobalShortcut/*, KAction::DefaultShortcut, KAction::NoAutoloading*/ );
    showMain->setText( i18n( "Minimize" ) );
    connect( showMain, SIGNAL( triggered( bool ) ), this, SLOT( toggleMainWindow() ) );

    KAction *enableUpdates = new KAction( i18n( "Enable Update Timer" ), this );
    enableUpdates->setCheckable( true );
    actionCollection()->addAction( QLatin1String( "choqok_enable_updates" ), enableUpdates );
    enableUpdates->setShortcut( KShortcut( Qt::CTRL | Qt::Key_U ) );
    enableUpdates->setGlobalShortcutAllowed( true );
    connect( enableUpdates, SIGNAL( toggled( bool ) ), this, SLOT( setTimeLineUpdatesEnabled( bool ) ) );

    KAction *enableNotify = new KAction( i18n( "Enable Notifications" ), this );
    enableNotify->setCheckable( true );
    actionCollection()->addAction( QLatin1String( "choqok_enable_notify" ), enableNotify );
    enableNotify->setShortcut( KShortcut( Qt::CTRL | Qt::Key_N ) );
    enableNotify->setGlobalShortcutAllowed( true );
    connect( enableNotify, SIGNAL( toggled( bool ) ), this, SLOT( setNotificationsEnabled( bool ) ) );

    KAction *clearAvatarCache = new KAction(KIcon("edit-clear"), i18n( "Clear Avatar cache" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_clear_avatar_cache" ), clearAvatarCache );
    QString tip = i18n( "You have to restart Choqok to load avatars again" );
    clearAvatarCache->setToolTip(tip);
    clearAvatarCache->setStatusTip(tip);
    connect( clearAvatarCache, SIGNAL( triggered() ), MediaManager::self(), SLOT(clearAvatarCache()) );

    ///SysTray Actions:
    sysIcon->contextMenu()->addAction( newTwit );
    sysIcon->contextMenu()->addAction( nowListening );

    sysIcon->contextMenu()->addAction( actUpdate );
    sysIcon->contextMenu()->addSeparator();

    connect( enableUpdates, SIGNAL( toggled( bool ) ), sysIcon, SLOT( setTimeLineUpdatesEnabled( bool ) ) );
    sysIcon->contextMenu()->addAction( enableUpdates );
    sysIcon->setTimeLineUpdatesEnabled( enableUpdates->isChecked() );
    sysIcon->contextMenu()->addAction( enableNotify );
    sysIcon->contextMenu()->addAction( prefs );

    connect(sysIcon,SIGNAL(wheelEvent(const QWheelEvent&)),this,SLOT(nextTab(const QWheelEvent&)));
    sysIcon->show();
}

void MainWindow::setupQuickTweet()
{
    quickWidget = new QuickTwit( this );
    quickWidget->setAttribute(Qt::WA_DeleteOnClose, false);
    connect( quickWidget, SIGNAL( sigNotify( const QString&, const  QString&, const  QString& ) ),
             this, SLOT( systemNotify( const QString&, const QString&, const QString& ) ) );
    connect( quickWidget, SIGNAL( sigStatusUpdated( bool ) ), sysIcon, SLOT( slotStatusUpdated( bool ) ) );
}

void MainWindow::postQuickTwit(const QString &text)
{
    if(!quickWidget)
        setupQuickTweet();
    if ( quickWidget->isVisible() ) {
        quickWidget->hide();
    } else {
        if( !text.isEmpty() )
            quickWidget->setText(text);
        quickWidget->showFocusedOnNewStatusField();
    }
}

void MainWindow::postNowListening()
{
    QDBusInterface remoteApp( "org.kde.amarok", "/Player", "org.freedesktop.MediaPlayer" );
    QDBusReply< QMap<QString, QVariant> > reply = remoteApp.call( "GetMetadata" );
    QVariantMap trackInfo = reply.value();
    QString text = Settings::nowListening();
    text.replace("%track%", trackInfo["tracknumber"].toString());
    text.replace("%title%", trackInfo["title"].toString());
    text.replace("%album%", trackInfo["album"].toString());
    text.replace("%artist%", trackInfo["artist"].toString());
    text.replace("%year%", trackInfo["year"].toString());
    text.replace("%genre%", trackInfo["genre"].toString());
    postQuickTwit(text);
}

void MainWindow::systemNotify( const QString &title, const QString &message, const QString &iconUrl )
{
    if ( Settings::notifyType() == SettingsBase::LibNotify ) {//Libnotify!
        QString msg = message;
        msg = msg.replace( "<br/>", "\n" );
        QString libnotifyCmd = QString( "notify-send -t " ) +
        QString::number( Settings::notifyInterval() * 1000 ) +
        QString( " -u low -i " + iconUrl + " \"" ) + title +
        QString( "\" \"" ) + msg + QString( "\"" );
        QProcess::execute( libnotifyCmd );
    } else {
        KNotification *notif = new KNotification( "notify", this );
        notif->setText( message );
        //         notify->setPixmap(mainWin-);
//         notif->setFlags( KNotification::RaiseWidgetOnActivation | KNotification::Persistent );
        notif->sendEvent();
//         QTimer::singleShot( Settings::notifyInterval()*1000, notif, SLOT( close() ) );
    }
}

void MainWindow::hideEvent( QHideEvent * event )
{
    Q_UNUSED(event);
    if( !this->isVisible() ) {
        emit setUnreadStatusesToReadState();
    }
}

void MainWindow::optionsPreferences()
{
    kDebug();

    if ( KConfigDialog::showDialog( "settings" ) )  {
        return;
    }

    KConfigDialog *dialog = new KConfigDialog( this, "settings", Settings::self() );

    QWidget *generalSettingsDlg = new QWidget;
    ui_prefs_base.setupUi( generalSettingsDlg );
    dialog->addPage( generalSettingsDlg, i18n( "General" ), "configure" );

    Accounts *accountsSettingsDlg = new Accounts( this );
    dialog->addPage( accountsSettingsDlg, i18n( "Accounts" ), "user-properties" );

    QWidget *appearsSettingsDlg = new QWidget;
    ui_appears_base.setupUi( appearsSettingsDlg );
    dialog->addPage( appearsSettingsDlg, i18n( "Appearance" ), "format-stroke-color" );

    AdvancedConfig *advancedSettingsDlg = new AdvancedConfig( this );
    dialog->addPage( advancedSettingsDlg, i18n("Advanced"), "applications-utilities");

    connect( dialog, SIGNAL( settingsChanged( QString ) ), this, SLOT( settingsChanged() ) );

    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->resize(Settings::configDialogSize());
    dialog->show();
}

void MainWindow::search(int type, const QString& query)
{
    kDebug();
    TimeLineWidget * tmp = qobject_cast<TimeLineWidget *>( mainWidget->widget( mainWidget->currentIndex() ) );

    SearchWindow* searchWin = new SearchWindow( tmp->currentAccount(), 0 );

    connect( searchWin, SIGNAL( forwardReply( const QString&, qulonglong, bool ) ),
             tmp, SLOT( prepareReply( const QString&, qulonglong, bool ) ) );
    connect( searchWin, SIGNAL(forwardReTweet(const QString&)),
             tmp, SLOT( reTweet(const QString&) ) );
    connect( searchWin, SIGNAL( forwardFavorited( qulonglong, bool ) ),
             tmp->getBackend(), SLOT( requestFavorited( qulonglong, bool ) ) );
    connect( this, SIGNAL( updateSearchResults() ),
             searchWin, SLOT( updateSearchResults() ) );
    connect( timelineTimer, SIGNAL( timeout() ),
             searchWin, SLOT( autoUpdateSearchResults() ) );
    connect( this, SIGNAL( updateSearchLimits() ), searchWin, SLOT( updateNumPages() ) );

    searchWin->init(type, query);
}

void MainWindow::settingsChanged()
{
    kDebug();
    if ( AccountManager::self()->accounts().count() < 1 ) {
        if ( KMessageBox::questionYesNo( this, i18n( "<qt>In order to use this program you need at \
least one account on <a href='http://identi.ca'>Identi.ca</a> or \
<a href='http://twitter.com'>Twitter.com</a> services.<br/>Would you like to add your account now?</qt>" )
                                       ) == KMessageBox::Yes ) {
            AccountsWizard *dia = new AccountsWizard( QString(), this );
            dia->setAttribute( Qt::WA_DeleteOnClose );
            dia->show();
        }
    }

    QWidget *w = qobject_cast< QWidget* >(sender());
    if( w ) {
        Settings::setConfigDialogSize(w->size());
    }

    if ( Settings::isCustomUi() ) {
    StatusWidget::setStyle( Settings::newStatusForeColor() , Settings::newStatusBackColor(),
                            Settings::defaultForeColor() , Settings::defaultBackColor());
    } else {
    QPalette p = window()->palette();
    StatusWidget::setStyle( p.color(QPalette::WindowText) , p.color(QPalette::Window).lighter() ,
                            p.color(QPalette::WindowText) , p.color(QPalette::Window));
    }
//     qApp->setStyleSheet(StatusWidget::getColoredStyle());

    int count = mainWidget->count();
    for ( int i = 0; i < count; ++i ) {
        qobject_cast<TimeLineWidget *>( mainWidget->widget( i ) )->settingsChanged();
    }
    if ( Settings::notifyType() == SettingsBase::NoNotify ) {
        actionCollection()->action( "choqok_enable_notify" )->setChecked( false );
    } else {
        actionCollection()->action( "choqok_enable_notify" )->setChecked( true );
    }
    if ( Settings::updateInterval() > 0 ) {
        timelineTimer->setInterval( Settings::updateInterval() *60000 );
        timelineTimer->start();
//         kDebug()<<"timelineTimer started";
        actionCollection()->action( "choqok_enable_updates" )->setChecked( true );
    } else {
        timelineTimer->stop();
//         kDebug()<<"timelineTimer stoped";
        actionCollection()->action( "choqok_enable_updates" )->setChecked( false );
    }

    emit updateSearchLimits();
}

void MainWindow::notify( const QString &message, bool isPermanent )
{
    if ( isPermanent ) {
        statusBar()->showMessage( message );
    } else {
        statusBar()->showMessage( message, TIMEOUT );
    }
}

void MainWindow::quitApp()
{
    kDebug();
    Settings::setPosition( pos() );
    timelineTimer->stop();
    Settings::self()->writeConfig();
    deleteLater();
}

void MainWindow::disableApp()
{
    kDebug();
    timelineTimer->stop();
//     kDebug()<<"timelineTimer stoped";
    actionCollection()->action( "update_timeline" )->setEnabled( false );
    actionCollection()->action( "choqok_new_twit" )->setEnabled( false );
    actionCollection()->action( "choqok_search" )->setEnabled( false );
    actionCollection()->action( "choqok_mark_read" )->setEnabled( false );
    actionCollection()->action( "choqok_now_listening" )->setEnabled( false );
}

void MainWindow::enableApp()
{
    kDebug();
    if ( Settings::updateInterval() > 0 ) {
        timelineTimer->start();
//         kDebug()<<"timelineTimer started";
    }
    actionCollection()->action( "update_timeline" )->setEnabled( true );
    actionCollection()->action( "choqok_new_twit" )->setEnabled( true );
    actionCollection()->action( "choqok_search" )->setEnabled( true );
    actionCollection()->action( "choqok_mark_read" )->setEnabled( true );
    actionCollection()->action( "choqok_now_listening" )->setEnabled( true );
}

void MainWindow::addAccountTimeLine( const Account & account, bool isStartup )
{
    kDebug() << "Alias: " << account.alias() << "Service :" << account.serviceName();

    TimeLineWidget *widget = new TimeLineWidget( account, this );
    widget->layout()->setContentsMargins( 0, 0, 0, 0 );

    connect( widget, SIGNAL( sigSetUnread( int ) ), sysIcon, SLOT( slotSetUnread( int ) ) );
    connect( widget, SIGNAL( systemNotify( const QString&, const QString&, const QString& ) ),
             this, SLOT( systemNotify( const QString&, const QString&, const QString& ) ) );
    connect( widget, SIGNAL( notify( const QString&, bool ) ), this, SLOT( notify( const QString&, bool ) ) );
    connect( widget, SIGNAL( showMe() ), this, SLOT( showTimeLine() ) );
//     connect(widget, SIGNAL(sigStatusUpdated(bool)), this, SIGNAL(sigStatusUpdated(bool)));

    connect( this, SIGNAL( updateTimeLines() ), widget, SLOT( updateTimeLines() ) );
    connect( this, SIGNAL( abortPostNewStatus() ), widget, SLOT( abortPostNewStatus() ) );
    connect( this, SIGNAL( setUnreadStatusesToReadState() ), widget, SLOT( setUnreadStatusesToReadState() ) );

    connect( widget, SIGNAL( sigSetUnreadOnMainWin( int ) ), this, SLOT( setNumOfUnreadOnMainWin( int ) ) );
    connect( widget, SIGNAL(sigSearch(int,QString)),this,SLOT(search(int,QString)));

    mainWidget->addTab( widget, account.alias() );

    QTimer::singleShot( 500, widget, SLOT( updateTimeLines() ) );
    if ( !isStartup ) {
        QTimer::singleShot( 1000, widget, SLOT(reloadFriendsList()) );
    }
    enableApp();
    if( mainWidget->count() > 1)
        mainWidget->setTabBarHidden(false);
    else
        mainWidget->setTabBarHidden(true);
}

void MainWindow::loadAccounts()
{
    kDebug();
    QList<Account> ac = AccountManager::self()->accounts();
    QListIterator<Account> it( ac );
    while ( it.hasNext() ) {
        Account current = it.next();
        addAccountTimeLine( current, true );
    }
    if ( ac.count() > 0 ) {
        enableApp();
    } else {
        disableApp();
    }
}

void MainWindow::removeAccountTimeLine( const QString & alias )
{
    kDebug();
    int count = mainWidget->count();
    for ( int i = 0; i < count; ++i ) {
        TimeLineWidget * tmp = qobject_cast<TimeLineWidget *>( mainWidget->widget( i ) );
        if ( tmp->currentAccount().alias() == alias ) {
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

void MainWindow::setNumOfUnreadOnMainWin( int unread )
{
//     kDebug()<<unread;
    TimeLineWidget *subWidget = qobject_cast<TimeLineWidget*>( sender() );
    QString text;
    if ( unread <= 0 ) {
        text = subWidget->currentAccount().alias();
    } else {
        text = i18nc( "account, unread","%1(%2)", subWidget->currentAccount().alias(), unread );
    }
    mainWidget->setTabText( mainWidget->indexOf( subWidget ), text );
}

void MainWindow::showTimeLine()
{
    mainWidget->setCurrentWidget( qobject_cast<TimeLineWidget*>( sender() ) );
    if ( !this->isVisible() )
        this->show();
    this->raise();
}

void MainWindow::setTimeLineUpdatesEnabled( bool isEnabled )
{
    kDebug();
    if ( isEnabled ) {
        if( mPrevUpdateInterval > 0 )
            Settings::setUpdateInterval( mPrevUpdateInterval );
        timelineTimer->start( Settings::updateInterval() *60000 );
//         kDebug()<<"timelineTimer started";
    } else {
        mPrevUpdateInterval = Settings::updateInterval();
        timelineTimer->stop();
//         kDebug()<<"timelineTimer stoped";
        Settings::setUpdateInterval( 0 );
    }
}

void MainWindow::setNotificationsEnabled( bool isEnabled )
{
    kDebug();
    if ( isEnabled ) {
        Settings::setNotifyType( (SettingsBase::NotifyType) mPrevNotifyType );
    } else {
        mPrevNotifyType = Settings::notifyType();
        Settings::setNotifyType( SettingsBase::NoNotify );
    }
}

void MainWindow::toggleMainWindow()
{
    if( this->isVisible() )
        hide();
    else
        show();
}

QSize MainWindow::sizeHint() const
{
    return QSize( 350, 380 );
}

#include "mainwindow.moc"
