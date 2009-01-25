/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#include "mainwindow.h"
#include "settings.h"
#include "timelinewidget.h"
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

#include "constants.h"
#include "accounts.h"
#include "accountmanager.h"
#include "accountswizard.h"

MainWindow::MainWindow()
        : KXmlGuiWindow()
{
    kDebug();

    this->setAttribute ( Qt::WA_DeleteOnClose, false );

    mainWidget = new KTabWidget ( this );

    setCentralWidget ( mainWidget );
    setupActions();
    statusBar()->show();
    notify ( i18n("Initializing choqoK, please be patient...") );
    setupGUI();

    timelineTimer = new QTimer ( this );
//     timelineTimer->setInterval ( Settings::updateInterval() *60000 );
//     timelineTimer->start();
    mPrevNotifyType = 1;
    mPrevUpdateInterval = 10;

    connect ( timelineTimer, SIGNAL ( timeout() ), this, SIGNAL ( updateTimeLines() ) );
    connect ( AccountManager::self(), SIGNAL(accountAdded(const Account&)), this, SLOT(addAccountTimeLine(const Account&)) );
    connect ( AccountManager::self(), SIGNAL(accountRemoved(const QString&)), this, SLOT(removeAccountTimeLine(const QString&)) );
    settingsChanged();
    QTimer::singleShot( 0, this, SLOT(loadAccounts()) );
}

MainWindow::~MainWindow()
{
    kDebug();
    timelineTimer->stop();
    Settings::self()->writeConfig();
}

void MainWindow::setupActions()
{
    KStandardAction::quit ( qApp, SLOT ( quit() ), actionCollection() );
    connect ( qApp, SIGNAL ( aboutToQuit() ), this, SLOT ( quitApp() ) );
    KStandardAction::preferences ( this, SLOT ( optionsPreferences() ), actionCollection() );

    KAction *actUpdate = new KAction ( KIcon ( "view-refresh" ), i18n ( "Update timelines" ), this );
    actionCollection()->addAction ( QLatin1String ( "update_timeline" ), actUpdate );
    actUpdate->setShortcut ( Qt::Key_F5 );
    actUpdate->setGlobalShortcutAllowed ( true );
    KShortcut updateGlobalShortcut ( Qt::CTRL | Qt::META | Qt::Key_F5 );
//     updateGlobalShortcut.setAlternate ( Qt::MetaModifier | Qt::Key_F5 );
    actUpdate->setGlobalShortcut ( updateGlobalShortcut );
    connect ( actUpdate, SIGNAL ( triggered ( bool ) ), this, SIGNAL ( updateTimeLines() ) );

    KAction *newTwit = new KAction ( KIcon ( "document-new" ), i18n ( "Quick Tweet" ), this );
    actionCollection()->addAction ( QLatin1String ( "choqok_new_twit" ), newTwit );
    newTwit->setShortcut ( KShortcut(Qt::CTRL | Qt::Key_T) );
    newTwit->setGlobalShortcutAllowed ( true );
    KShortcut quickTwitGlobalShortcut ( Qt::CTRL | Qt::META | Qt::Key_T );
//     quickTwitGlobalShortcut.setAlternate ( Qt::MetaModifier | Qt::Key_T );
    newTwit->setGlobalShortcut ( quickTwitGlobalShortcut );

    KAction *showMain = new KAction(this);
    actionCollection()->addAction( QLatin1String( "toggle_mainwin" ), showMain);
    showMain->setShortcut( KShortcut(Qt::CTRL + Qt::Key_C) );
    KShortcut toggleMainGlobalShortcut( Qt::CTRL | Qt::META | Qt::Key_C );
    showMain->setGlobalShortcutAllowed( true );
    showMain->setGlobalShortcut( toggleMainGlobalShortcut/*, KAction::DefaultShortcut, KAction::NoAutoloading*/ );
    if(this->isVisible())
        showMain->setText(i18n("Minimize"));
    else
        showMain->setText(i18n("Restore"));
    connect(showMain, SIGNAL(triggered( bool )), this, SLOT(toggleMainWindowVisibility()));

    KAction *enableUpdates = new KAction ( i18n ( "Enable update timer" ), this );
    enableUpdates->setCheckable( true );
    actionCollection()->addAction ( QLatin1String ( "choqok_enable_updates" ), enableUpdates );
    enableUpdates->setShortcut ( KShortcut(Qt::CTRL | Qt::Key_U) );
    enableUpdates->setGlobalShortcutAllowed ( true );
    connect ( enableUpdates, SIGNAL(toggled( bool )), this, SLOT(setTimeLineUpdatesEnabled(bool)) );
//     KShortcut quickTwitGlobalShortcut ( Qt::CTRL | Qt::META | Qt::Key_T );
//     quickTwitGlobalShortcut.setAlternate ( Qt::MetaModifier | Qt::Key_T );
//     newTwit->setGlobalShortcut ( quickTwitGlobalShortcut );

    KAction *enableNotify = new KAction ( i18n ( "Enable notifications" ), this );
    enableNotify->setCheckable( true );
    actionCollection()->addAction ( QLatin1String ( "choqok_enable_notify" ), enableNotify );
    enableNotify->setShortcut ( KShortcut(Qt::CTRL | Qt::Key_N) );
    enableNotify->setGlobalShortcutAllowed ( true );
    connect ( enableNotify, SIGNAL(toggled( bool )), this, SLOT(setNotificationsEnabled(bool)) );
}

void MainWindow::toggleMainWindowVisibility()
{
    if(this->isVisible()){
        this->close();
        actionCollection()->action("toggle_mainwin")->setText(i18n("&Restore"));
    } else {
        this->show();
        actionCollection()->action("toggle_mainwin")->setText(i18n("&Minimize"));
    }
}

void MainWindow::optionsPreferences()
{
    kDebug();

    if ( KConfigDialog::showDialog ( "settings" ) )  {
        return;
    }

    KConfigDialog *dialog = new KConfigDialog ( this, "settings", Settings::self() );

    QWidget *generalSettingsDlg = new QWidget;
    ui_prefs_base.setupUi ( generalSettingsDlg );
    dialog->addPage ( generalSettingsDlg, i18n ( "General" ), "configure" );

    Accounts *accountsSettingsDlg = new Accounts ( this );
    dialog->addPage ( accountsSettingsDlg, i18n ( "Accounts" ), "user-properties" );

    QWidget *appearsSettingsDlg = new QWidget;
    ui_appears_base.setupUi ( appearsSettingsDlg );
    dialog->addPage ( appearsSettingsDlg, i18n ( "Appearances" ), "format-stroke-color" );

    connect ( dialog, SIGNAL ( settingsChanged ( QString ) ), this, SLOT ( settingsChanged() ) );

    dialog->setAttribute ( Qt::WA_DeleteOnClose );
    dialog->show();
}

void MainWindow::settingsChanged()
{
    kDebug();
    if ( AccountManager::self()->accounts().count() < 1 ){
        if(KMessageBox::questionYesNo( this, i18n("<qt>In order to use this app you need at \
least one account on <a href='http://identi.ca'>Identi.ca</a> or \
<a href='http://twitter.com'>Twitter.com</a> services.<br/>Would you like to add your account now?</qt>")
                                     ) == KMessageBox::Yes ) {
            AccountsWizard *dia = new AccountsWizard(QString(), this);
            dia->setAttribute( Qt::WA_DeleteOnClose );
            dia->show();
                                     }
    }
    timelineTimer->setInterval ( Settings::updateInterval() *60000 );

    int count = mainWidget->count();
    for ( int i=0; i<count; ++i  ){
        qobject_cast<TimeLineWidget *>( mainWidget->widget( i ) )->settingsChanged();
    }
    if(Settings::notifyType() == 0){
        actionCollection()->action( "choqok_enable_notify" )->setChecked( false );
    } else {
        actionCollection()->action( "choqok_enable_notify" )->setChecked( true );
    }
    if(Settings::updateInterval() > 2){
        timelineTimer->start();
        actionCollection()->action( "choqok_enable_updates" )->setChecked( true );
    } else {
        kDebug()<<"Updating disabeld.";
        timelineTimer->stop();
        actionCollection()->action( "choqok_enable_updates" )->setChecked( false );
    }
}

void MainWindow::notify ( const QString &message )
{
    statusBar()->showMessage ( message, TIMEOUT );
}

void MainWindow::keyPressEvent ( QKeyEvent * e )
{
//     if ( e->key() == Qt::Key_Escape ) {
// //     if(txtNewStatus->isEnabled()){
// //       this->hide();
// //     } else {
// //       txtNewStatus->setEnabled(true);
// //       twitter->abortPostNewStatus();
//         ///TODO ^ Current visible TimelineWidget abort!
// //     }
//     } else {
        KXmlGuiWindow::keyPressEvent ( e );
//     }
}

void MainWindow::quitApp()
{
    deleteLater();
}

void MainWindow::loadConfigurations()
{
    kDebug();
}

bool MainWindow::queryClose()
{
    kDebug();
    emit setUnreadStatusesToReadState();
    return true;
}

void MainWindow::disableApp()
{
    kDebug();
    timelineTimer->stop();
    actionCollection()->action ( "update_timeline" )->setEnabled ( false );
    actionCollection()->action ( "choqok_new_twit" )->setEnabled ( false );
}

void MainWindow::enableApp()
{
    kDebug();
    timelineTimer->start();
    actionCollection()->action ( "update_timeline" )->setEnabled ( true );
    actionCollection()->action ( "choqok_new_twit" )->setEnabled ( true );
}

void MainWindow::addAccountTimeLine(const Account & account)
{
    kDebug()<<"Alias: "<<account.alias() <<"Service :"<<account.serviceName();
    TimeLineWidget *widget = new TimeLineWidget(account, this);
    widget->layout()->setContentsMargins( 0, 0, 0, 0);

    connect(widget, SIGNAL(sigSetUnread(int)), this, SIGNAL(sigSetUnread(int)));
    connect(widget, SIGNAL(systemNotify(const QString&, const QString&, const QString&)),
           this, SIGNAL(systemNotify(const QString&, const QString&, const QString&)));
    connect(widget, SIGNAL(notify(const QString&)), this, SLOT(notify(const QString&)));
    connect(widget, SIGNAL(showMe()), this, SLOT(showTimeLine()));

    connect(this, SIGNAL(updateTimeLines()), widget, SLOT(updateTimeLines()));
    connect(this, SIGNAL(abortPostNewStatus()), widget, SLOT(abortPostNewStatus()));
    connect(this, SIGNAL(setUnreadStatusesToReadState()), widget, SLOT(setUnreadStatusesToReadState()));

    connect(widget, SIGNAL(sigSetUnreadOnMainWin(int)), this, SLOT(setNumOfUnreadOnMainWin(int)));

    mainWidget->addTab(widget, account.alias());

    QTimer::singleShot(500, widget, SLOT(updateTimeLines()));
    enableApp();
}

void MainWindow::loadAccounts()
{
    kDebug();
    QList<Account> ac = AccountManager::self()->accounts();
    QListIterator<Account> it(ac);
    while(it.hasNext()){
        Account current = it.next();
        addAccountTimeLine(current);
    }
    if(ac.count()>0){
        enableApp();
    } else {
        disableApp();
    }
}

void MainWindow::removeAccountTimeLine(const QString & alias)
{
    kDebug();
    int count = mainWidget->count();
    for(int i=0; i<count; ++i){
        TimeLineWidget * tmp = qobject_cast<TimeLineWidget *>( mainWidget->widget( i ) );
        if(tmp->currentAccount().alias() == alias){
            mainWidget->removeTab( i );
            if(mainWidget->count()<1)
                disableApp();
            tmp->setRemoved( true );
            tmp->deleteLater();
            return;
        }
    }
}

void MainWindow::setNumOfUnreadOnMainWin(int unread)
{
//     kDebug()<<unread;
    TimeLineWidget *subWidget = qobject_cast<TimeLineWidget*>(sender());
    QString text;
    if( unread <= 0){
        text = subWidget->currentAccount().alias();
    } else {
        text = subWidget->currentAccount().alias() + i18n("(%1)", unread);
    }
    mainWidget->setTabText(mainWidget->indexOf(subWidget), text);
}

void MainWindow::showTimeLine()
{
    mainWidget->setCurrentWidget( qobject_cast<TimeLineWidget*>(sender()) );
    if(!this->isVisible())
        this->show();
    this->raise();
}

void MainWindow::setTimeLineUpdatesEnabled(bool isEnabled)
{
    kDebug();
    if(isEnabled){
        Settings::setUpdateInterval( mPrevUpdateInterval );
        timelineTimer->start( Settings::updateInterval() *60000 );
    } else {
        mPrevUpdateInterval = Settings::updateInterval();
        timelineTimer->stop();
        Settings::setUpdateInterval( 2 );
    }
}

void MainWindow::setNotificationsEnabled(bool isEnabled)
{
    kDebug();
    if(isEnabled){
        Settings::setNotifyType( mPrevNotifyType );
    } else {
        mPrevNotifyType = Settings::notifyType();
        Settings::setNotifyType( 0 );
    }
}


#include "mainwindow.moc"
