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

#include "mainwindow.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotifyConfigWidget>
#include <ksettings/Dialog>
#include <KStandardAction>
#include <KXMLGUIFactory>

#include "accountmanager.h"
#include "choqokappearancesettings.h"
#include "choqokapplication.h"
#include "choqokbehaviorsettings.h"
#include "choqokdebug.h"
#include "choqoktools.h"
#include "choqokuiglobal.h"
#include "mediamanager.h"
#include "microblogwidget.h"
#include "pluginmanager.h"
#include "postwidget.h"
#include "quickpost.h"
#include "systrayicon.h"
#include "uploadmediadialog.h"

const char *mainButtonStyleSheet = "QPushButton{\
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

MainWindow::MainWindow(ChoqokApplication *application)
    : Choqok::UI::MainWindow(), sysIcon(0), quickWidget(0), s_settingsDialog(0),
      m_splash(0), choqokMainButton(0), app(application),
      microblogCounter(0), choqokMainButtonVisible(false)
{
    qCDebug(CHOQOK);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_QuitOnClose, false);

    timelineTimer = new QTimer(this);
    setWindowTitle(i18n("Choqok"));
    connect(mainWidget, SIGNAL(currentChanged(int)), SLOT(slotCurrentBlogChanged(int)));
    setCentralWidget(mainWidget);

    setupActions();
    updateSysTray();
    statusBar()->show();
    setupGUI();

    if (Choqok::BehaviorSettings::updateInterval() > 0) {
        mPrevUpdateInterval = Choqok::BehaviorSettings::updateInterval();
    } else {
        mPrevUpdateInterval = 10;
    }

    connect(timelineTimer, SIGNAL(timeout()), this, SIGNAL(updateTimelines()));
    connect(this, SIGNAL(markAllAsRead()), SLOT(slotMarkAllAsRead()));
    connect(Choqok::AccountManager::self(), SIGNAL(accountAdded(Choqok::Account*)),
            this, SLOT(addBlog(Choqok::Account*)));
    connect(Choqok::AccountManager::self(), SIGNAL(accountRemoved(QString)),
            this, SLOT(removeBlog(QString)));
    connect(Choqok::AccountManager::self(), SIGNAL(allAccountsLoaded()),
            SLOT(loadAllAccounts()));

    connect(Choqok::PluginManager::self(), SIGNAL(pluginLoaded(Choqok::Plugin*)),
            this, SLOT(newPluginAvailable(Choqok::Plugin*)));

    QTimer::singleShot(0, Choqok::PluginManager::self(), SLOT(loadAllPlugins()));
//     Choqok::AccountManager::self()->loadAllAccounts();
    QTimer::singleShot(0, Choqok::AccountManager::self(), SLOT(loadAllAccounts()));

    connect(this, SIGNAL(updateTimelines()), SLOT(slotUpdateTimelines()));

    QPoint pos = Choqok::BehaviorSettings::position();
    if (pos.x() != -1 && pos.y() != -1) {
        move(pos);
    }
    actionCollection()->action(QLatin1String("choqok_hide_menubar"))->setChecked(menuBar()->isHidden());
}

MainWindow::~MainWindow()
{
    qCDebug(CHOQOK);
}

void MainWindow::loadAllAccounts()
{
    qCDebug(CHOQOK);

    if (Choqok::BehaviorSettings::showSplashScreen()) {
        const QPixmap splashpix(QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("images/splash_screen.png")));
        if (splashpix.isNull()) {
            qCCritical(CHOQOK) << "Splash screen pixmap is NULL!";
        } else {
            m_splash = new QSplashScreen(splashpix, Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
            m_splash->show();
        }
    }

    settingsChanged();
    QList<Choqok::Account *> accList = Choqok::AccountManager::self()->accounts();
    int count = microblogCounter = accList.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            addBlog(accList.at(i), true);
        }
        qCDebug(CHOQOK) << "All accounts loaded.";
        if (Choqok::BehaviorSettings::updateInterval() > 0) {
            QTimer::singleShot(500, this, SIGNAL(updateTimelines()));
        }
    } else {
        m_splash->finish(this);
        delete m_splash;
        m_splash = 0;
    }
    ChoqokApplication::setStartingUp(false);
    createQuickPostDialog();
}

void MainWindow::newPluginAvailable(Choqok::Plugin *plugin)
{
    qCDebug(CHOQOK);
    guiFactory()->addClient(plugin);
}

void MainWindow::nextTab(int delta, Qt::Orientation orientation)
{
    if (!isVisible()) {
        return;
    }
    QTabWidget *widget = 0;
    switch (orientation) {
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
    if (!widget) {
        return;
    }

    int count = widget->count();
    int index = widget->currentIndex();
    int page;
    if (delta > 0) {
        page = index > 0 ? index - 1 : count - 1;
    } else {
        page = index < count - 1 ? index + 1 : 0;
    }
    widget->setCurrentIndex(page);
}

void MainWindow::setupActions()
{
    actQuit = KStandardAction::quit(this, SLOT(slotQuit()), actionCollection());
    prefs = KStandardAction::preferences(this, SLOT(slotConfigChoqok()), actionCollection());

    actUpdate = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")), i18n("Update Timelines"), this);
    actionCollection()->addAction(QLatin1String("update_timeline"), actUpdate);
    actionCollection()->setDefaultShortcut(actUpdate, QKeySequence(Qt::Key_F5));
    KGlobalAccel::setGlobalShortcut(actUpdate, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_F5));
    connect(actUpdate, SIGNAL(triggered(bool)), this, SIGNAL(updateTimelines()));

    newTwit = new QAction(QIcon::fromTheme(QLatin1String("document-new")), i18n("Quick Post"), this);
    actionCollection()->addAction(QLatin1String("choqok_new_post"), newTwit);
    actionCollection()->setDefaultShortcut(newTwit, QKeySequence(Qt::CTRL | Qt::Key_T));
    KGlobalAccel::setGlobalShortcut(newTwit, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_T));
    connect(newTwit, SIGNAL(triggered(bool)), this, SLOT(triggerQuickPost()));

    QAction *markRead = new QAction(QIcon::fromTheme(QLatin1String("mail-mark-read")), i18n("Mark All As Read"), this);
    actionCollection()->addAction(QLatin1String("choqok_mark_as_read"), markRead);
    actionCollection()->setDefaultShortcut(markRead, QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(markRead, SIGNAL(triggered(bool)), this, SIGNAL(markAllAsRead()));

    showMain = new QAction(this);
    actionCollection()->addAction(QLatin1String("toggle_mainwin"), showMain);
    KGlobalAccel::setGlobalShortcut(showMain, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_C));
    if (this->isVisible()) {
        showMain->setText(i18nc("@action", "Minimize"));
    } else {
        showMain->setText(i18nc("@action", "Restore"));
    }
    connect(showMain, SIGNAL(triggered(bool)), this, SLOT(toggleMainWindow()));

    QAction *act = KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()),
                   actionCollection());
    actionCollection()->addAction(QLatin1String("settings_notifications"), act);

    enableUpdates = new QAction(i18n("Enable Update Timer"), this);
    enableUpdates->setCheckable(true);
    actionCollection()->addAction(QLatin1String("choqok_enable_updates"), enableUpdates);
    actionCollection()->setDefaultShortcut(enableUpdates, QKeySequence(Qt::CTRL | Qt::Key_U));
    connect(enableUpdates, SIGNAL(toggled(bool)), this, SLOT(setTimeLineUpdatesEnabled(bool)));

    QAction *enableNotify = new QAction(i18n("Enable Notifications"), this);
    enableNotify->setCheckable(true);
    actionCollection()->addAction(QLatin1String("choqok_enable_notify"), enableNotify);
    actionCollection()->setDefaultShortcut(enableNotify, QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(enableNotify, SIGNAL(toggled(bool)), this, SLOT(setNotificationsEnabled(bool)));

    QAction *hideMenuBar = new QAction(i18n("Hide Menubar"), this);
    hideMenuBar->setCheckable(true);
    actionCollection()->addAction(QLatin1String("choqok_hide_menubar"), hideMenuBar);
    actionCollection()->setDefaultShortcut(hideMenuBar, QKeySequence(Qt::ControlModifier | Qt::Key_M));
    connect(hideMenuBar, SIGNAL(toggled(bool)), menuBar(), SLOT(setHidden(bool)));
    connect(hideMenuBar, SIGNAL(toggled(bool)), this, SLOT(slotShowSpecialMenu(bool)));

    QAction *clearAvatarCache = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear Avatar Cache"), this);
    actionCollection()->addAction(QLatin1String("choqok_clear_avatar_cache"), clearAvatarCache);
    QString tip = i18n("You have to restart Choqok to load avatars again");
    clearAvatarCache->setToolTip(tip);
    clearAvatarCache->setStatusTip(tip);
    connect(clearAvatarCache, SIGNAL(triggered()),
            Choqok::MediaManager::self(), SLOT(clearImageCache()));

    QAction *uploadMedium = new QAction(QIcon::fromTheme(QLatin1String("arrow-up")), i18n("Upload Medium..."), this);
    actionCollection()->addAction(QLatin1String("choqok_upload_medium"), uploadMedium);
    connect(uploadMedium, SIGNAL(triggered(bool)), this, SLOT(slotUploadMedium()));

    QAction *donate = new QAction(QIcon::fromTheme(QLatin1String("help-donate")), i18n("Donate"), this);
    actionCollection()->addAction(QLatin1String("choqok_donate"), donate);
    connect(donate, SIGNAL(triggered(bool)), this, SLOT(slotDonate()));
}

void MainWindow::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void MainWindow::createQuickPostDialog()
{
    quickWidget = new Choqok::UI::QuickPost(this);
    Choqok::UI::Global::setQuickPostWidget(quickWidget);
    quickWidget->setAttribute(Qt::WA_DeleteOnClose, false);
    if (sysIcon) {
        connect(quickWidget, SIGNAL(newPostSubmitted(Choqok::JobResult)),
                sysIcon, SLOT(slotJobDone(Choqok::JobResult)));
    }
    emit(quickPostCreated());
}

void MainWindow::triggerQuickPost()
{
    if (Choqok::AccountManager::self()->accounts().isEmpty()) {
        KMessageBox::error(this, i18n("No account created. You have to create an account before being able to make posts."));
        return;
    }
    if (!quickWidget) {
        createQuickPostDialog();
    }
    if (quickWidget->isVisible()) {
        quickWidget->hide();
    } else {
        quickWidget->show();
    }
}

void MainWindow::slotConfigChoqok()
{
    if (!s_settingsDialog) {
        s_settingsDialog = new KSettings::Dialog(this);
    }
    s_settingsDialog->show();
    connect(Choqok::BehaviorSettings::self(), SIGNAL(configChanged()),
            SLOT(slotBehaviorConfigChanged()));
    connect(Choqok::AppearanceSettings::self(), SIGNAL(configChanged()),
            SLOT(slotAppearanceConfigChanged()));
}

void MainWindow::settingsChanged()
{
    qCDebug(CHOQOK);
    if (Choqok::AccountManager::self()->accounts().count() < 1) {
        if (KMessageBox::questionYesNo(this, i18n("In order to use Choqok you need \
an account at one of the supported micro-blogging services.\n\
Would you like to add your account now?")) == KMessageBox::Yes) {
            slotConfigChoqok();
        }
    }
    slotAppearanceConfigChanged();
    slotBehaviorConfigChanged();
}

void MainWindow::slotAppearanceConfigChanged()
{
    if (Choqok::AppearanceSettings::isCustomUi()) {
        Choqok::UI::PostWidget::setStyle(Choqok::AppearanceSettings::unreadForeColor() ,
                                         Choqok::AppearanceSettings::unreadBackColor(),
                                         Choqok::AppearanceSettings::readForeColor() ,
                                         Choqok::AppearanceSettings::readBackColor() ,
                                         Choqok::AppearanceSettings::ownForeColor() ,
                                         Choqok::AppearanceSettings::ownBackColor(),
                                         Choqok::AppearanceSettings::font());
    } else {
        QPalette p = window()->palette();
        Choqok::UI::PostWidget::setStyle(p.color(QPalette::WindowText) , p.color(QPalette::Window).lighter() ,
                                         p.color(QPalette::WindowText) , p.color(QPalette::Window) ,
                                         p.color(QPalette::WindowText) , p.color(QPalette::Window),
                                         font());
    }
    int count = mainWidget->count();
    for (int i = 0; i < count; ++i) {
        qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->widget(i))->settingsChanged();
    }
}

void MainWindow::updateSysTray()
{
    if (Choqok::BehaviorSettings::enableSysTray()) {
        if (!sysIcon) {
            sysIcon = new SysTrayIcon(this);
//            sysIcon->show();

            ///SysTray Actions:
            sysIcon->contextMenu()->addAction(newTwit);
//            sysIcon->contextMenu()->addAction( uploadMedium );
            sysIcon->contextMenu()->addAction(actUpdate);
            sysIcon->contextMenu()->addSeparator();
            connect(enableUpdates, SIGNAL(toggled(bool)), sysIcon,
                    SLOT(setTimeLineUpdatesEnabled(bool)));
            sysIcon->contextMenu()->addAction(enableUpdates);
            sysIcon->setTimeLineUpdatesEnabled(enableUpdates->isChecked());
//           sysIcon->contextMenu()->addAction( enableNotify );
            sysIcon->contextMenu()->addAction(prefs);

            sysIcon->contextMenu()->addSeparator();
            sysIcon->contextMenu()->addAction(showMain);
            sysIcon->contextMenu()->addAction(actQuit);
//            connect( sysIcon, SIGNAL(quitSelected()), this, SLOT(slotQuit()) );
            connect(sysIcon, SIGNAL(scrollRequested(int,Qt::Orientation)),
                    this, SLOT(nextTab(int,Qt::Orientation)));
        }
    } else {
        if (sysIcon) {
            if (isHidden()) {
                show();
            }

            delete sysIcon;
            sysIcon = 0;
        }
    }
}

void MainWindow::slotBehaviorConfigChanged()
{
    if (Choqok::BehaviorSettings::notifyEnabled()) {
        actionCollection()->action(QLatin1String("choqok_enable_notify"))->setChecked(true);
    } else {
        actionCollection()->action(QLatin1String("choqok_enable_notify"))->setChecked(false);
    }
    if (Choqok::BehaviorSettings::updateInterval() > 0) {
        timelineTimer->setInterval(Choqok::BehaviorSettings::updateInterval() * 60000);
        timelineTimer->start();
        actionCollection()->action(QLatin1String("choqok_enable_updates"))->setChecked(true);
    } else {
        timelineTimer->stop();
        actionCollection()->action(QLatin1String("choqok_enable_updates"))->setChecked(false);
    }

    updateSysTray();
}

void MainWindow::slotQuit()
{
    qCDebug(CHOQOK);
    Choqok::BehaviorSettings::setPosition(pos());
    timelineTimer->stop();
    Choqok::BehaviorSettings::self()->save();
    app->quitChoqok();
}

void MainWindow::disableApp()
{
    qCDebug(CHOQOK);
    timelineTimer->stop();
//     qCDebug(CHOQOK)<<"timelineTimer stoped";
    actionCollection()->action(QLatin1String("update_timeline"))->setEnabled(false);
    actionCollection()->action(QLatin1String("choqok_new_post"))->setEnabled(false);
//     actionCollection()->action( "choqok_search" )->setEnabled( false );
    actionCollection()->action(QLatin1String("choqok_mark_as_read"))->setEnabled(false);
//     actionCollection()->action( "choqok_now_listening" )->setEnabled( false );
}

void MainWindow::enableApp()
{
    qCDebug(CHOQOK);
    if (Choqok::BehaviorSettings::updateInterval() > 0) {
        timelineTimer->start();
//         qCDebug(CHOQOK)<<"timelineTimer started";
    }
    actionCollection()->action(QLatin1String("update_timeline"))->setEnabled(true);
    actionCollection()->action(QLatin1String("choqok_new_post"))->setEnabled(true);
//     actionCollection()->action( "choqok_search" )->setEnabled( true );
    actionCollection()->action(QLatin1String("choqok_mark_as_read"))->setEnabled(true);
//     actionCollection()->action( "choqok_now_listening" )->setEnabled( true );
}

void MainWindow::addBlog(Choqok::Account *account, bool isStartup)
{
    qCDebug(CHOQOK) << "Adding new Blog, Alias:" << account->alias() << "Blog:" << account->microblog()->serviceName();

    Choqok::UI::MicroBlogWidget *widget = account->microblog()->createMicroBlogWidget(account, this);
    connect(widget, SIGNAL(loaded()), SLOT(oneMicroblogLoaded()));
    connect(widget, SIGNAL(updateUnreadCount(int,int)), SLOT(slotUpdateUnreadCount(int,int)));
    widget->initUi();

//     connect( widget, SIGNAL(sigSetUnread(int)), sysIcon, SLOT(slotSetUnread(int)) );
    /*connect( widget, SIGNAL(showStatusMessage(QString,bool)),
             this, SLOT(showStatusMessage(QString,bool)) )*/;
    connect(widget, SIGNAL(showMe()), this, SLOT(showBlog()));

    connect(this, SIGNAL(updateTimelines()), widget, SLOT(updateTimelines()));
    connect(this, SIGNAL(markAllAsRead()), widget, SLOT(markAllAsRead()));
    connect(this, SIGNAL(removeOldPosts()), widget, SLOT(removeOldPosts()));
//     qCDebug(CHOQOK)<<"Plugin Icon: "<<account->microblog()->pluginIcon();
    mainWidget->addTab(widget, QIcon::fromTheme(account->microblog()->pluginIcon()), account->alias());

    if (!isStartup) {
        QTimer::singleShot(1500, widget, SLOT(updateTimelines()));
    }
    enableApp();
    updateTabbarHiddenState();
}

void MainWindow::removeBlog(const QString &alias)
{
    qCDebug(CHOQOK);
    int count = mainWidget->count();
    for (int i = 0; i < count; ++i) {
        Choqok::UI::MicroBlogWidget *tmp = qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->widget(i));
        if (tmp->currentAccount()->alias() == alias) {
            mainWidget->removeTab(i);
            if (mainWidget->count() < 1) {
                disableApp();
            }
            tmp->deleteLater();
            updateTabbarHiddenState();
            return;
        }
    }
}

void MainWindow::updateTabbarHiddenState()
{
    if (mainWidget->count() <= 1 && !choqokMainButtonVisible) {
        mainWidget->tabBar()->hide();
    } else {
        mainWidget->tabBar()->show();
    }
}

void MainWindow::slotUpdateUnreadCount(int change, int sum)
{
    qCDebug(CHOQOK) << "Change:" << change << "Sum:" << sum;
    Choqok::UI::MicroBlogWidget *wd = qobject_cast<Choqok::UI::MicroBlogWidget *>(sender());

    if (sysIcon) {
        sysIcon->updateUnreadCount(change);
    }

    if (sum > 0) {
        setWindowTitle(i18n("Choqok (%1)", sum));
    } else {
        setWindowTitle(i18n("Choqok"));
    }

    if (wd) {
        int tabIndex = mainWidget->indexOf(wd);
        if (tabIndex == -1) {
            return;
        }
        if (sum > 0) {
            mainWidget->setTabText(tabIndex, wd->currentAccount()->alias() + QStringLiteral("(%1)").arg(sum));
        } else {
            mainWidget->setTabText(tabIndex, wd->currentAccount()->alias());
        }
    }
}

void MainWindow::showBlog()
{
    mainWidget->setCurrentWidget(qobject_cast<QWidget *>(sender()));
    if (!this->isVisible()) {
        this->show();
    }
    this->raise();
}

void MainWindow::setTimeLineUpdatesEnabled(bool isEnabled)
{
    qCDebug(CHOQOK);
    if (isEnabled) {
        if (mPrevUpdateInterval > 0) {
            Choqok::BehaviorSettings::setUpdateInterval(mPrevUpdateInterval);
        }
        Q_EMIT updateTimelines();
        timelineTimer->start(Choqok::BehaviorSettings::updateInterval() * 60000);
//         qCDebug(CHOQOK)<<"timelineTimer started";
    } else {
        mPrevUpdateInterval = Choqok::BehaviorSettings::updateInterval();
        timelineTimer->stop();
//         qCDebug(CHOQOK)<<"timelineTimer stoped";
        Choqok::BehaviorSettings::setUpdateInterval(0);
    }
}

void MainWindow::setNotificationsEnabled(bool isEnabled)
{
    qCDebug(CHOQOK);
    if (isEnabled) {
        Choqok::BehaviorSettings::setNotifyEnabled(true);
    } else {
        Choqok::BehaviorSettings::setNotifyEnabled(false);
    }
}

void MainWindow::toggleMainWindow()
{
    if (this->isVisible()) {
        hide();
    } else {
        show();
    }
}

void MainWindow::hideEvent(QHideEvent *event)
{
    Choqok::UI::MainWindow::hideEvent(event);
    showMain->setText(i18n("Restore"));
}

void MainWindow::showEvent(QShowEvent *event)
{
    Choqok::UI::MainWindow::showEvent(event);
    showMain->setText(i18n("Minimize"));
}

void MainWindow::slotMarkAllAsRead()
{
    setWindowTitle(i18n("Choqok"));
    if (sysIcon) {
        sysIcon->resetUnreadCount();
    }
    int count = mainWidget->count();
    for (int i = 0; i < count; ++i) {
        Choqok::UI::MicroBlogWidget *wd = qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->widget(i));
        mainWidget->setTabText(i, wd->currentAccount()->alias());
    }
}

void MainWindow::slotCurrentBlogChanged(int)
{
    Choqok::UI::MicroBlogWidget *wd = qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->currentWidget());
    if (wd) {
        wd->setFocus();
        Q_EMIT currentMicroBlogWidgetChanged(wd);
    }
}

void MainWindow::oneMicroblogLoaded()
{
    qCDebug(CHOQOK);
    --microblogCounter;
    if (microblogCounter < 1) {
        //Everything loaded, So splash screen should be deleted!
        delete m_splash;
        m_splash = 0;
        if (Choqok::BehaviorSettings::showMainWinOnStart()) {
            this->show();
        }
    } else {
        if (m_splash) {
            m_splash->showMessage(QString());    //Workaround for Qt 4.8 splash bug
        }
    }
}

void MainWindow::slotUpdateTimelines()
{
    if (Choqok::AccountManager::self()->accounts().count()) {
        showStatusMessage(i18n("Loading timelines..."));
    }
}

void MainWindow::slotUploadMedium()
{
    QPointer<Choqok::UI::UploadMediaDialog> dlg = new Choqok::UI::UploadMediaDialog(this);
    dlg->show();
}

void MainWindow::slotShowSpecialMenu(bool show)
{
    if (show) {
        if (!choqokMainButton) {
            choqokMainButton = new QPushButton(QIcon::fromTheme(QLatin1String("choqok")), QString(), mainWidget);
            QMenu *menu = new QMenu(i18n("Choqok"), choqokMainButton);
            menu->addAction(actionCollection()->action(QLatin1String("choqok_new_post")));
            menu->addAction(actionCollection()->action(QLatin1String("update_timeline")));
            menu->addAction(actionCollection()->action(QLatin1String("choqok_mark_as_read")));
            menu->addAction(actionCollection()->action(QLatin1String("choqok_upload_medium")));
            menu->addSeparator();
            menu->addAction(actionCollection()->action(QLatin1String("choqok_enable_updates")));
            menu->addAction(actionCollection()->action(QLatin1String("choqok_hide_menubar")));
            menu->addAction(prefs);
            menu->addSeparator();
            menu->addAction(actionCollection()->action(QLatin1String("choqok_donate")));
            menu->addSeparator();
            menu->addAction(showMain);
            menu->addAction(actQuit);
            choqokMainButton->setMenu(menu);
        }
        mainWidget->setCornerWidget(choqokMainButton/*, Qt::TopLeftCorner*/);
        choqokMainButton->show();
        choqokMainButtonVisible = true;
    } else {
        choqokMainButtonVisible = false;
        mainWidget->setCornerWidget(0/*, Qt::TopLeftCorner*/);
    }
    updateTabbarHiddenState();
}

void MainWindow::slotDonate()
{
    Choqok::openUrl(QUrl(QLatin1String("http://choqok.gnufolks.org/about/contribute/")));
}

