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
    timelineTimer->setInterval ( Settings::updateInterval() *60000 );
    timelineTimer->start();

    connect ( timelineTimer, SIGNAL ( timeout() ), this, SIGNAL ( updateTimeLines() ) );
    connect ( AccountManager::self(), SIGNAL(accountAdded(const Account&)), this, SLOT(addAccountTimeLine(const Account&)) );
    connect ( AccountManager::self(), SIGNAL(accountRemoved(const QString&)), this, SLOT(removeAccountTimeLine(const QString&)) );
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
    KShortcut updateGlobalShortcut ( Qt::ControlModifier | Qt::MetaModifier | Qt::Key_F5 );
    updateGlobalShortcut.setAlternate ( Qt::MetaModifier | Qt::Key_F5 );
    actUpdate->setGlobalShortcut ( updateGlobalShortcut );
    connect ( actUpdate, SIGNAL ( triggered ( bool ) ), this, SIGNAL ( updateTimeLines() ) );

    KAction *newTwit = new KAction ( this );
    newTwit->setIcon ( KIcon ( "document-new" ) );
    newTwit->setText ( i18n ( "Quick Tweet" ) );
    actionCollection()->addAction ( QLatin1String ( "choqok_new_twit" ), newTwit );
    newTwit->setShortcut ( Qt::ControlModifier | Qt::Key_T );
    newTwit->setGlobalShortcutAllowed ( true );
    KShortcut quickTwitGlobalShortcut ( Qt::ControlModifier | Qt::MetaModifier | Qt::Key_T );
    quickTwitGlobalShortcut.setAlternate ( Qt::MetaModifier | Qt::Key_T );
    newTwit->setGlobalShortcut ( quickTwitGlobalShortcut );
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
//   connect(dialog, SIGNAL(settingsChanged(QString)), twitter, SLOT(settingsChanged()));********************
//   currentUsername = Settings::username();
    dialog->setAttribute ( Qt::WA_DeleteOnClose );
    dialog->show();
}

void MainWindow::settingsChanged()
{
    kDebug();
    ///TODO Check if there's at least one account? and there isn't any so disable app
//   if(currentUsername != Settings::username()){
//     setUnreadStatusesToReadState();
//     reloadTimeLineLists();
//   }
//   setDefaultDirection();
    timelineTimer->setInterval ( Settings::updateInterval() *60000 );

    ///TODO Call for All TimeLineWidgets settingsChanged() function.
    /// or simply add all of them to SIGNAL on optionsPreferences function. :)
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

    timelineTimer->setInterval ( Settings::updateInterval() *60000 );
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
    kDebug()<<"Alias: "<<account.alias <<"Service :"<<account.serviceName;
    TimeLineWidget *widget = new TimeLineWidget(account, this);
    
    connect(widget, SIGNAL(sigSetUnread(int)), this, SIGNAL(sigSetUnread(int)));
    connect(widget, SIGNAL(systemNotify(const QString&, const QString&, const QString&)),
           this, SIGNAL(systemNotify(const QString&, const QString&, const QString&)));
    connect(widget, SIGNAL(notify(const QString&)), this, SLOT(notify(const QString&)));
    
    connect(this, SIGNAL(updateTimeLines()), widget, SLOT(updateTimeLines()));
    connect(this, SIGNAL(abortPostNewStatus()), widget, SLOT(abortPostNewStatus()));
    connect(this, SIGNAL(setUnreadStatusesToReadState()), widget, SLOT(setUnreadStatusesToReadState()));
    
    connect(widget, SIGNAL(sigSetUnreadOnMainWin(int)), this, SLOT(setNumOfUnreadOnMainWin(int)));
    
    mainWidget->addTab(widget, account.alias);
    
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
        if(tmp->currentAccount().alias == alias){
            mainWidget->removeTab( i );
            if(mainWidget->count()<1)
                disableApp();
            return;
        }
    }
}

// void MainWindow::updateAccountTimeLine(const Account & account)
// {
//     
// }

void MainWindow::setNumOfUnreadOnMainWin(int unread)
{
//     kDebug()<<unread;
    TimeLineWidget *subWidget = qobject_cast<TimeLineWidget*>(sender());
    QString text;
    if( unread <= 0){
        text = subWidget->currentAccount().alias;
    } else {
        text = subWidget->currentAccount().alias + i18n("(%1)", unread);
    }
    mainWidget->setTabText(mainWidget->indexOf(subWidget), text);
}

#include "mainwindow.moc"
