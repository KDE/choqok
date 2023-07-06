/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mainwindow.h"

#include "config-choqok.h"

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

#if HAVE_KGLOBALACCEL
#include <KGlobalAccel>
#endif
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotifyConfigWidget>
#include <KCMultiDialog>
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

MainWindow::MainWindow(ChoqokApplication *application)
    : Choqok::UI::MainWindow(), sysIcon(nullptr), quickWidget(nullptr), s_settingsDialog(nullptr),
      m_splash(nullptr), choqokMainButton(nullptr), app(application),
      microblogCounter(0), choqokMainButtonVisible(false)
{
    qCDebug(CHOQOK);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_QuitOnClose, false);

    timelineTimer = new QTimer(this);
    setWindowTitle(i18n("Choqok"));
    connect(mainWidget, &QTabWidget::currentChanged, this, &MainWindow::slotCurrentBlogChanged);
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

    connect(timelineTimer, &QTimer::timeout, this, &MainWindow::updateTimelines);
    connect(this, &MainWindow::markAllAsRead, this, &MainWindow::slotMarkAllAsRead);
    connect(Choqok::AccountManager::self(), SIGNAL(accountAdded(Choqok::Account*)),
            this, SLOT(addBlog(Choqok::Account*)));
    connect(Choqok::AccountManager::self(), &Choqok::AccountManager::accountRemoved,
            this, &MainWindow::removeBlog);
    connect(Choqok::AccountManager::self(), &Choqok::AccountManager::allAccountsLoaded,
            this, &MainWindow::loadAllAccounts);

    connect(Choqok::PluginManager::self(), &Choqok::PluginManager::pluginLoaded,
            this, &MainWindow::newPluginAvailable);

    QTimer::singleShot(0, Choqok::PluginManager::self(), &Choqok::PluginManager::loadAllPlugins);
//     Choqok::AccountManager::self()->loadAllAccounts();
    QTimer::singleShot(0, Choqok::AccountManager::self(), &Choqok::AccountManager::loadAllAccounts);

    connect(this, &MainWindow::updateTimelines, this, &MainWindow::slotUpdateTimelines);

    QPoint pos = Choqok::BehaviorSettings::position();
    if (pos.x() != -1 && pos.y() != -1) {
        move(pos);
    }
    actionCollection()->action(QLatin1String("choqok_hide_menubar"))->setChecked(menuBar()->isHidden());
}

MainWindow::~MainWindow()
{
    qCDebug(CHOQOK);

    disconnect(this, &MainWindow::markAllAsRead, this, &MainWindow::slotMarkAllAsRead);
    disconnect(this, &MainWindow::updateTimelines, this, &MainWindow::slotUpdateTimelines);
    disconnect(Choqok::BehaviorSettings::self(), &Choqok::BehaviorSettings::configChanged,
               this, &MainWindow::slotBehaviorConfigChanged);
    disconnect(Choqok::AppearanceSettings::self(), &Choqok::AppearanceSettings::configChanged,
               this, &MainWindow::slotAppearanceConfigChanged);
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
    const int count = microblogCounter = accList.count();
    if (count > 0) {
        for (Choqok::Account *ac: accList) {
            connect(ac, &Choqok::Account::status, this, &MainWindow::updateBlog);
            addBlog(ac, true);
        }
        qCDebug(CHOQOK) << "All accounts loaded.";
        if (Choqok::BehaviorSettings::updateInterval() > 0) {
            QTimer::singleShot(500, this, &MainWindow::updateTimelines);
        }
    } else {
        if (m_splash) {
            m_splash->finish(this);
            delete m_splash;
            m_splash = nullptr;
        }
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
    QTabWidget *widget = nullptr;
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
#if HAVE_KGLOBALACCEL
    KGlobalAccel::setGlobalShortcut(actUpdate, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_F5));
#endif
    connect(actUpdate, &QAction::triggered, this, &MainWindow::updateTimelines);

    newTwit = new QAction(QIcon::fromTheme(QLatin1String("document-new")), i18n("Quick Post"), this);
    actionCollection()->addAction(QLatin1String("choqok_new_post"), newTwit);
    actionCollection()->setDefaultShortcut(newTwit, QKeySequence(Qt::CTRL | Qt::Key_T));
#if HAVE_KGLOBALACCEL
    KGlobalAccel::setGlobalShortcut(newTwit, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_T));
#endif
    connect(newTwit, &QAction::triggered, this, &MainWindow::triggerQuickPost);

    QAction *markRead = new QAction(QIcon::fromTheme(QLatin1String("mail-mark-read")), i18n("Mark All As Read"), this);
    actionCollection()->addAction(QLatin1String("choqok_mark_as_read"), markRead);
    actionCollection()->setDefaultShortcut(markRead, QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(markRead, &QAction::triggered, this, &MainWindow::markAllAsRead);

    showMain = new QAction(this);
    actionCollection()->addAction(QLatin1String("toggle_mainwin"), showMain);
#if HAVE_KGLOBALACCEL
    KGlobalAccel::setGlobalShortcut(showMain, QKeySequence(Qt::CTRL | Qt::META | Qt::Key_C));
#endif
    if (this->isVisible()) {
        showMain->setText(i18nc("@action", "Minimize"));
    } else {
        showMain->setText(i18nc("@action", "Restore"));
    }
    connect(showMain, &QAction::triggered, this, &MainWindow::toggleMainWindow);

    QAction *act = KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()),
                   actionCollection());
    actionCollection()->addAction(QLatin1String("settings_notifications"), act);

    enableUpdates = new QAction(i18n("Enable Update Timer"), this);
    enableUpdates->setCheckable(true);
    actionCollection()->addAction(QLatin1String("choqok_enable_updates"), enableUpdates);
    actionCollection()->setDefaultShortcut(enableUpdates, QKeySequence(Qt::CTRL | Qt::Key_U));
    connect(enableUpdates, &QAction::toggled, this, &MainWindow::setTimeLineUpdatesEnabled);

    QAction *enableNotify = new QAction(i18n("Enable Notifications"), this);
    enableNotify->setCheckable(true);
    actionCollection()->addAction(QLatin1String("choqok_enable_notify"), enableNotify);
    actionCollection()->setDefaultShortcut(enableNotify, QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(enableNotify, &QAction::toggled, this, &MainWindow::setNotificationsEnabled);

    QAction *hideMenuBar = new QAction(i18n("Hide Menubar"), this);
    hideMenuBar->setCheckable(true);
    actionCollection()->addAction(QLatin1String("choqok_hide_menubar"), hideMenuBar);
    actionCollection()->setDefaultShortcut(hideMenuBar, QKeySequence(Qt::ControlModifier | Qt::Key_M));
    connect(hideMenuBar, &QAction::toggled, menuBar(), &QMenuBar::setHidden);
    connect(hideMenuBar, &QAction::toggled, this, &MainWindow::slotShowSpecialMenu);

    QAction *clearAvatarCache = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear Avatar Cache"), this);
    actionCollection()->addAction(QLatin1String("choqok_clear_avatar_cache"), clearAvatarCache);
    QString tip = i18n("You have to restart Choqok to load avatars again");
    clearAvatarCache->setToolTip(tip);
    clearAvatarCache->setStatusTip(tip);
    connect(clearAvatarCache, &QAction::triggered, Choqok::MediaManager::self(), &Choqok::MediaManager::clearImageCache);

    QAction *uploadMedium = new QAction(QIcon::fromTheme(QLatin1String("arrow-up")), i18n("Upload Medium..."), this);
    actionCollection()->addAction(QLatin1String("choqok_upload_medium"), uploadMedium);
    connect(uploadMedium, &QAction::triggered, this, &MainWindow::slotUploadMedium);
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
        connect(quickWidget, &Choqok::UI::QuickPost::newPostSubmitted, sysIcon, &SysTrayIcon::slotJobDone);
    }
    Q_EMIT quickPostCreated();
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
        s_settingsDialog = new KCMultiDialog(this);
        for (const auto& plugin : KPluginMetaData::findPlugins(QStringLiteral("choqok_kcms"))) {
            s_settingsDialog->addModule(plugin);
        }
    }
    s_settingsDialog->show();
    connect(Choqok::BehaviorSettings::self(), &Choqok::BehaviorSettings::configChanged,
            this, &MainWindow::slotBehaviorConfigChanged);
    connect(Choqok::AppearanceSettings::self(), &Choqok::AppearanceSettings::configChanged,
            this, &MainWindow::slotAppearanceConfigChanged);
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

    for (int i = 0; i < mainWidget->count(); ++i) {
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
            connect(enableUpdates, &QAction::toggled, sysIcon, &SysTrayIcon::setTimeLineUpdatesEnabled);
            sysIcon->contextMenu()->addAction(enableUpdates);
            sysIcon->setTimeLineUpdatesEnabled(enableUpdates->isChecked());
//           sysIcon->contextMenu()->addAction( enableNotify );
            sysIcon->contextMenu()->addAction(prefs);

            sysIcon->contextMenu()->addSeparator();
            sysIcon->contextMenu()->addAction(showMain);
            sysIcon->contextMenu()->addAction(actQuit);
//            connect( sysIcon, SIGNAL(quitSelected()), this, SLOT(slotQuit()) );
            connect(sysIcon, &SysTrayIcon::scrollRequested, this, &MainWindow::nextTab);
        }
    } else {
        if (sysIcon) {
            if (isHidden()) {
                show();
            }

            delete sysIcon;
            sysIcon = nullptr;
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

    if (!account->isEnabled()) {
        oneMicroblogLoaded();
        return;
    }

    Choqok::UI::MicroBlogWidget *widget = account->microblog()->createMicroBlogWidget(account, this);
    connect(widget, &Choqok::UI::MicroBlogWidget::loaded,            this, &MainWindow::oneMicroblogLoaded);
    connect(widget, &Choqok::UI::MicroBlogWidget::updateUnreadCount, this, &MainWindow::slotUpdateUnreadCount);
    widget->initUi();

    connect(widget, &Choqok::UI::MicroBlogWidget::showMe, this, &MainWindow::showBlog);

    connect(this, &MainWindow::updateTimelines, widget, &Choqok::UI::MicroBlogWidget::updateTimelines);
    connect(this, &MainWindow::markAllAsRead,   widget, &Choqok::UI::MicroBlogWidget::markAllAsRead);
    connect(this, &MainWindow::removeOldPosts,  widget, &Choqok::UI::MicroBlogWidget::removeOldPosts);

    mainWidget->addTab(widget, QIcon::fromTheme(account->microblog()->pluginIcon()), account->alias());

    if (!isStartup) {
        QTimer::singleShot(1500, widget, &Choqok::UI::MicroBlogWidget::updateTimelines);
    }
    enableApp();
    updateTabbarHiddenState();
}

void MainWindow::updateBlog(Choqok::Account *account, bool enabled)
{
    if (!enabled) {
        removeBlog(account->alias());
    } else {
        addBlog(account);
    }
}

void MainWindow::removeBlog(const QString &alias)
{
    qCDebug(CHOQOK);
    for (int i = 0; i < mainWidget->count(); ++i) {
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

    int accountsSum = 0;
    for (int i = 0; i < mainWidget->tabBar()->count(); ++i) {
        Choqok::UI::MicroBlogWidget *tab = qobject_cast<Choqok::UI::MicroBlogWidget *>(mainWidget->widget(i));

        accountsSum += tab->unreadCount();

        if (wd == tab) {
            if (sum > 0) {
                mainWidget->setTabText(i, wd->currentAccount()->alias() + QStringLiteral(" (%1)").arg(sum));
            } else {
                mainWidget->setTabText(i, wd->currentAccount()->alias());
            }
        }
    }

    if (accountsSum > 0) {
        setWindowTitle(i18n("Choqok (%1)", accountsSum));
    } else {
        setWindowTitle(i18n("Choqok"));
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
    for (int i = 0; i < mainWidget->count(); ++i) {
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
        m_splash = nullptr;
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
            menu->addAction(showMain);
            menu->addAction(actQuit);
            choqokMainButton->setMenu(menu);
        }
        mainWidget->setCornerWidget(choqokMainButton/*, Qt::TopLeftCorner*/);
        choqokMainButton->show();
        choqokMainButtonVisible = true;
    } else {
        choqokMainButtonVisible = false;
        mainWidget->setCornerWidget(nullptr/*, Qt::TopLeftCorner*/);
    }
    updateTabbarHiddenState();
}
