/*
    This file is part of choqoK, the KDE micro-blogging client

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
#include <QKeyEvent>
#include <QScrollBar>
#include <QMenu>
#include <KNotification>


MainWindow::MainWindow()
    : KXmlGuiWindow()
{
    kDebug();
    quickWidget = 0;
    timelineTimer = new QTimer( this );

    this->setAttribute( Qt::WA_DeleteOnClose, false );

    mainWidget = new KTabWidget( this );

    setCentralWidget( mainWidget );
    sysIcon = new SysTrayIcon(this);
//     setupQuickTweet();
    setupActions();
    statusBar()->show();
    notify( i18n( "Initializing choqoK, please be patient..." ) );
    setupGUI();

//     timelineTimer->setInterval ( Settings::updateInterval() *60000 );
//     timelineTimer->start();
    if ( Settings::notifyType() == SettingsBase::NoNotify )
        mPrevNotifyType = 1;
    else
        mPrevNotifyType = Settings::notifyType();
    if ( Settings::updateInterval() > 2 )
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
    qApp->setStyleSheet(QString()); //crashes if qApp->styleSheet() != QString(), maybe Qt/KDE bug?
    kDebug();
}

void MainWindow::setupActions()
{
    KStandardAction::quit( qApp, SLOT( quit() ), actionCollection() );
    connect( qApp, SIGNAL( aboutToQuit() ), this, SLOT( quitApp() ) );
    KAction *prefs = KStandardAction::preferences( this, SLOT( optionsPreferences() ), actionCollection() );

    KAction *actUpdate = new KAction( KIcon( "view-refresh" ), i18n( "Update timelines" ), this );
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

    KAction *newSearch = new KAction( KIcon( "edit-find" ), i18n( "Search" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_search" ), newSearch );
    newSearch->setShortcut( KShortcut( Qt::CTRL | Qt::Key_F ) );
    newSearch->setGlobalShortcutAllowed( false );
    connect( newSearch, SIGNAL( triggered( bool ) ), this, SLOT( search() ) );

    KAction *markRead = new KAction( KIcon( "mail-mark-read" ), i18n( "Mark All As Read" ), this );
    actionCollection()->addAction( QLatin1String( "choqok_mark_read" ), markRead );
    markRead->setShortcut( KShortcut( Qt::CTRL | Qt::Key_R ) );
    actUpdate->setGlobalShortcutAllowed( false );
    connect( markRead, SIGNAL( triggered( bool ) ), this, SIGNAL( setUnreadStatusesToReadState() ) );

    KAction *showMain = new KAction( this );
    actionCollection()->addAction( QLatin1String( "toggle_mainwin" ), showMain );
    KShortcut toggleMainGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_C );
    showMain->setGlobalShortcutAllowed( true );
    showMain->setGlobalShortcut( toggleMainGlobalShortcut/*, KAction::DefaultShortcut, KAction::NoAutoloading*/ );
    showMain->setText( i18n( "Minimize" ) );
    connect( showMain, SIGNAL( triggered( bool ) ), this, SLOT( hide() ) );

    KAction *enableUpdates = new KAction( i18n( "Enable update timer" ), this );
    enableUpdates->setCheckable( true );
    actionCollection()->addAction( QLatin1String( "choqok_enable_updates" ), enableUpdates );
    enableUpdates->setShortcut( KShortcut( Qt::CTRL | Qt::Key_U ) );
    enableUpdates->setGlobalShortcutAllowed( true );
    connect( enableUpdates, SIGNAL( toggled( bool ) ), this, SLOT( setTimeLineUpdatesEnabled( bool ) ) );

    KAction *enableNotify = new KAction( i18n( "Enable notifications" ), this );
    enableNotify->setCheckable( true );
    actionCollection()->addAction( QLatin1String( "choqok_enable_notify" ), enableNotify );
    enableNotify->setShortcut( KShortcut( Qt::CTRL | Qt::Key_N ) );
    enableNotify->setGlobalShortcutAllowed( true );
    connect( enableNotify, SIGNAL( toggled( bool ) ), this, SLOT( setNotificationsEnabled( bool ) ) );

    ///SysTray Actions:
    sysIcon->contextMenu()->addAction( newTwit );
//     sysIcon->contextMenu()->addAction( newSearch );

    sysIcon->contextMenu()->addAction( actUpdate );
    sysIcon->contextMenu()->addSeparator();

    connect( enableUpdates, SIGNAL( toggled( bool ) ), sysIcon, SLOT( setTimeLineUpdatesEnabled( bool ) ) );
    sysIcon->contextMenu()->addAction( enableUpdates );
    sysIcon->setTimeLineUpdatesEnabled( enableUpdates->isChecked() );
    sysIcon->show();

    sysIcon->contextMenu()->addAction( enableNotify );
    sysIcon->contextMenu()->addAction( prefs );
}

void MainWindow::setupQuickTweet()
{
    quickWidget = new QuickTwit( this );
    connect( quickWidget, SIGNAL( sigNotify( const QString&, const  QString&, const  QString& ) ),
             this, SLOT( systemNotify( const QString&, const QString&, const QString& ) ) );
    connect( quickWidget, SIGNAL( sigStatusUpdated( bool ) ), sysIcon, SLOT( slotStatusUpdated( bool ) ) );
}

void MainWindow::postQuickTwit()
{
    if(!quickWidget)
        setupQuickTweet();
    if ( quickWidget->isVisible() ) {
        quickWidget->hide();
    } else {
        quickWidget->showFocusedOnNewStatusField();
    }
}

void MainWindow::systemNotify( const QString &title, const QString &message, const QString &iconUrl )
{
    if ( Settings::notifyType() == SettingsBase::KNotify ) {//KNotify
        KNotification *notif = new KNotification( "notify", this );
        notif->setText( message );
        //         notify->setPixmap(mainWin-);
        notif->setFlags( KNotification::RaiseWidgetOnActivation | KNotification::Persistent );
        notif->sendEvent();
        QTimer::singleShot( Settings::notifyInterval()*1000, notif, SLOT( close() ) );

    } else if ( Settings::notifyType() == SettingsBase::LibNotify ) {//Libnotify!
        QString msg = message;
        msg = msg.replace( "<br/>", "\n" );
        QString libnotifyCmd = QString( "notify-send -t " ) +
        QString::number( Settings::notifyInterval() * 1000 ) +
        QString( " -u low -i " + iconUrl + " \"" ) + title +
        QString( "\" \"" ) + msg + QString( "\"" );
        QProcess::execute( libnotifyCmd );
    }
}

void MainWindow::hideEvent( QHideEvent * event )
{
    Q_UNUSED(event);
    emit setUnreadStatusesToReadState();
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
    dialog->addPage( appearsSettingsDlg, i18n( "Appearances" ), "format-stroke-color" );

    connect( dialog, SIGNAL( settingsChanged( QString ) ), this, SLOT( settingsChanged() ) );

    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->show();
}

void MainWindow::search()
{
    kDebug();
    TimeLineWidget * tmp = qobject_cast<TimeLineWidget *>( mainWidget->widget( mainWidget->currentIndex() ) );

    SearchWindow* searchWin = new SearchWindow( tmp->currentAccount(), 0 );

    connect( searchWin, SIGNAL( forwardReply( const QString&, uint, bool ) ),
             tmp, SLOT( prepareReply( const QString&, uint, bool ) ) );
    connect( searchWin, SIGNAL( forwardFavorited( uint, bool ) ),
             tmp->getBackend(), SLOT( requestFavorited( uint, bool ) ) );
    connect( this, SIGNAL( updateSearchResults() ),
             searchWin, SLOT( updateSearchResults() ) );
    connect( timelineTimer, SIGNAL( timeout() ),
             searchWin, SLOT( autoUpdateSearchResults() ) );

    searchWin->init();
}

void MainWindow::settingsChanged()
{
    kDebug();
    if ( AccountManager::self()->accounts().count() < 1 ) {
        if ( KMessageBox::questionYesNo( this, i18n( "<qt>In order to use this app you need at \
least one account on <a href='http://identi.ca'>Identi.ca</a> or \
<a href='http://twitter.com'>Twitter.com</a> services.<br/>Would you like to add your account now?</qt>" )
                                       ) == KMessageBox::Yes ) {
            AccountsWizard *dia = new AccountsWizard( QString(), this );
            dia->setAttribute( Qt::WA_DeleteOnClose );
            dia->show();
        }
    }
    timelineTimer->setInterval( Settings::updateInterval() *60000 );

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
    if ( Settings::updateInterval() > 2 ) {
        timelineTimer->start();
//         kDebug()<<"timelineTimer started";
        actionCollection()->action( "choqok_enable_updates" )->setChecked( true );
    } else {
        timelineTimer->stop();
//         kDebug()<<"timelineTimer stoped";
        actionCollection()->action( "choqok_enable_updates" )->setChecked( false );
    }
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

void MainWindow::loadConfigurations()
{
    kDebug();
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
}

void MainWindow::enableApp()
{
    kDebug();
    if ( Settings::updateInterval() > 2 ) {
        timelineTimer->start();
//         kDebug()<<"timelineTimer started";
    }
    actionCollection()->action( "update_timeline" )->setEnabled( true );
    actionCollection()->action( "choqok_new_twit" )->setEnabled( true );
    actionCollection()->action( "choqok_search" )->setEnabled( true );
    actionCollection()->action( "choqok_mark_read" )->setEnabled( true );
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

    mainWidget->addTab( widget, account.alias() );

    if ( !isStartup ) {
        QTimer::singleShot( 500, widget, SLOT( updateTimeLines() ) );
        QTimer::singleShot( 1000, widget, SLOT(reloadFriendsList()) );
    }
    enableApp();
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
//             tmp->setRemoved( true );
            tmp->deleteLater();
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
        Settings::setUpdateInterval( mPrevUpdateInterval );
        timelineTimer->start( Settings::updateInterval() *60000 );
//         kDebug()<<"timelineTimer started";
    } else {
        mPrevUpdateInterval = Settings::updateInterval();
        timelineTimer->stop();
//         kDebug()<<"timelineTimer stoped";
        Settings::setUpdateInterval( 2 );
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


#include "mainwindow.moc"
