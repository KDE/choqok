/*
 * MainWindow.cpp
 *
 * Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>
 */
#include "mainwindow.h"
#include "settings.h"
#include "statuswidget.h"

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
#include "statustextedit.h"
#include "mediamanagement.h"

MainWindow::MainWindow()
	: KXmlGuiWindow()
{
	kDebug();
    // accept dnd
//     setAcceptDrops(true);
	this->setAttribute(Qt::WA_DeleteOnClose, false);
	Settings::setLatestStatusId(0);
	Settings::self()->readConfig();
	twitter = new Backend(this);
	connect(twitter, SIGNAL(homeTimeLineRecived(QList< Status >&)), this, SLOT(homeTimeLinesRecived(QList< Status >&)));
	connect(twitter, SIGNAL(replyTimeLineRecived(QList< Status >&)), this, SLOT(replyTimeLineRecived(QList< Status >&)));
	connect(twitter, SIGNAL(sigPostNewStatusDone(bool)), this, SLOT(postingNewStatusDone(bool)));
	connect(twitter, SIGNAL(sigFavoritedDone(bool)), this, SLOT(requestFavoritedDone(bool)));
	connect(twitter, SIGNAL(sigDestroyDone(bool)), this, SLOT(requestDestroyDone(bool)));
    // tell the KXmlGuiWindow that this is indeed the main widget
	mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
	ui.homeLayout->setDirection(QBoxLayout::TopToBottom);
	ui.replyLayout->setDirection(QBoxLayout::TopToBottom);
	txtNewStatus = new StatusTextEdit(mainWidget);
	txtNewStatus->setObjectName("txtNewStatus");
	txtNewStatus->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	txtNewStatus->setMaximumHeight(70);
	txtNewStatus->setFocus(Qt::OtherFocusReason);
	txtNewStatus->setTabChangesFocus ( true );
	ui.inputFrame->layout()->addWidget(txtNewStatus);
	
	setCentralWidget(mainWidget);
	replyToStatusId = 0;

	setupActions();
	statusBar()->show();
	setupGUI();
	
	timelineTimer = new QTimer(this);
	timelineTimer->start();
		
	mediaMan = new MediaManagement(this);
	
	connect(timelineTimer, SIGNAL(timeout()), this, SLOT(updateTimeLines()));
	connect(txtNewStatus, SIGNAL(charsLeft(int)), this, SLOT(checkNewStatusCharactersCount(int)));
	connect(txtNewStatus, SIGNAL(returnPressed(QString&)), this, SLOT(postStatus(QString&)));
	connect(this, SIGNAL(sigSetUserImage(StatusWidget*)), this, SLOT(setUserImage(StatusWidget*)));
	connect(ui.toggleArrow, SIGNAL(clicked()), this, SLOT(toggleTwitFieldVisible()));
	
	loadConfigurations();
}

MainWindow::~MainWindow()
{
	kDebug();
	//TODO Save Status list
	timelineTimer->stop();
	Settings::self()->writeConfig();
}

void MainWindow::setupActions()
{
	KStandardAction::quit(qApp, SLOT(quit()), actionCollection());
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(quitApp()));
	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	KAction *actUpdate = new KAction(KIcon("view-refresh"), i18n("Update timeline"), this);
	actionCollection()->addAction(QLatin1String("update_timeline"), actUpdate);
	actUpdate->setShortcut(Qt::Key_F5);
	actUpdate->setGlobalShortcutAllowed(true);
	actUpdate->setGlobalShortcut(KShortcut(Qt::ControlModifier | Qt::MetaModifier | Qt::Key_F5));
	connect(actUpdate, SIGNAL(triggered( bool )), this, SLOT(updateTimeLines()));
	
	KAction *newTwit = new KAction(this);
	newTwit->setIcon(KIcon("document-new"));
	newTwit->setText(i18n("Quick Twit"));
	actionCollection()->addAction(QLatin1String("choqok_new_twit"), newTwit);
	newTwit->setShortcut(Qt::ControlModifier | Qt::Key_T);
	newTwit->setGlobalShortcutAllowed(true);
	newTwit->setGlobalShortcut(KShortcut(Qt::ControlModifier | Qt::MetaModifier | Qt::Key_T));
	
// 	KAction *toggleTwitField = new KAction(this);
// 	toggleTwitField->setShortcut(Qt::ControlModifier | Qt::Key_E);
// 	if(ui.inputFrame->isVisible())
// 		toggleTwitField->setText(i18n("Hide twit box"));
// 	else
// 		toggleTwitField->setText(i18n("Show twit box"));
// 	actionCollection()->addAction(QLatin1String("toggle_twit_field"), toggleTwitField);
// 	connect(toggleTwitField, SIGNAL(triggered( bool )), this, SLOT(actToggleTwitFieldVisible()));
}

void MainWindow::optionsPreferences()
{
	kDebug();
    // The preference dialog is derived from prefs_base.ui
    //
    // compare the names of the widgets in the .ui file
    // to the names of the variables in the .kcfg file
    //avoid to have 2 dialogs shown
    if ( KConfigDialog::showDialog( "settings" ) )  {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());
	
    QWidget *generalSettingsDlg = new QWidget;
    ui_prefs_base.setupUi(generalSettingsDlg);
    dialog->addPage(generalSettingsDlg, i18n("General"), "package_setting");
	
	QWidget *accountsSettingsDlg = new QWidget;
	ui_accounts_base.setupUi(accountsSettingsDlg);
	dialog->addPage(accountsSettingsDlg, i18n("Account"), "accounts_setting");
	
    connect(dialog, SIGNAL(settingsChanged(QString)), this, SLOT(settingsChanged()));
	currentUsername = Settings::username();
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->show();
}

void MainWindow::checkNewStatusCharactersCount(int numOfChars)
{
	if(numOfChars < 30){
		ui.lblCounter->setStyleSheet("QLabel {color: red}");
	} else {
		ui.lblCounter->setStyleSheet("QLabel {color: green}");
	}
	ui.lblCounter->setText(i18n("%1", numOfChars));
}

void MainWindow::settingsChanged()
{
	kDebug();
	if(currentUsername != Settings::username()){
		setUnreadStatusesToReadState();
		reloadTimeLineLists();
	}
	setDefaultDirection();
	timelineTimer->setInterval(Settings::updateInterval()*60000);
	if(Settings::hideTwitField()){
		ui.inputFrame->hide();
		ui.toggleArrow->setArrowType(Qt::LeftArrow);
	} else{
		ui.toggleArrow->setArrowType(Qt::DownArrow);
		ui.inputFrame->show();
		txtNewStatus->setFocus(Qt::OtherFocusReason);
	}
}

void MainWindow::notify(const QString &message)
{
	statusBar()->showMessage( message, TIMEOUT);
}

// void MainWindow::systemNotify(const QString title, const QString message, QString iconUrl)
// {
// 	switch(Settings::notifyType()){
// 			case 0:
// 				break;
// 			case 1://KNotify
// 				break;
// 			case 2://Libnotify!
// 				QString libnotifyCmd = QString("notify-send -t ") + QString::number(Settings::notifyInterval()*1000) + QString(" -u low -i "+ iconUrl +" \"") + title + QString("\" \"") + message + QString("\"");
// 				QProcess::execute(libnotifyCmd);
// 				break;
// 		}
// }

void MainWindow::updateTimeLines()
{
	kDebug();
	twitter->requestTimeLine(Backend::HomeTimeLine);
	twitter->requestTimeLine(Backend::ReplyTimeLine);
	if(Settings::latestStatusId()==0)
		isStartMode = true;
	else
		isStartMode = false;
	statusBar()->showMessage(i18n("Loading timelines..."));
}

void MainWindow::updateHomeTimeLine()
{
	kDebug();
	timelineTimer->start();
	twitter->requestTimeLine(Backend::HomeTimeLine);
	if(Settings::latestStatusId()==0)
		isStartMode = true;
	else
		isStartMode = false;
}

void MainWindow::homeTimeLinesRecived(QList< Status > & statusList)
{
	kDebug();
	statusBar()->showMessage(i18n("Latest friends timeline recived!"), TIMEOUT);
	int count = statusList.count();
	if(count==0){
		kDebug()<<"Status list is empty";
		statusBar()->showMessage(i18n("No new twits recived, The list is up to date."), TIMEOUT);
		return;
	} else {
		int count = statusList.count();
		kDebug()<<count<<" Statuses received.";
		addNewStatusesToUi(statusList, ui.homeLayout, &listHomeStatus);
		ui.homeScroll->verticalScrollBar()->setSliderPosition ( 0 );
		if(!isStartMode){
			unreadStatusInHome+=count;
			ui.tabs->setTabText(0, i18n("Home(%1)", unreadStatusInHome));
		}
	}
}

void MainWindow::replyTimeLineRecived(QList< Status > & statusList)
{
	kDebug();
	statusBar()->showMessage(i18n("Latest replies timeline recived!"), TIMEOUT);
	if(statusList.count()==0){
		kDebug()<<"Status list is empty";
		statusBar()->showMessage(i18n("No new twits received, The list is up to date."), TIMEOUT);
		return;
	}else {
		int count = statusList.count();
		kDebug()<<count<<" Statuses received.";
		addNewStatusesToUi(statusList, ui.replyLayout, &listReplyStatus, Backend::ReplyTimeLine);
		ui.replyScroll->verticalScrollBar()->setSliderPosition ( 0 );
		if(!isStartMode){
			unreadStatusInReply+=count;
			ui.tabs->setTabText(1, i18n("Reply(%1)", unreadStatusInReply));
		}
	}
}

void MainWindow::addNewStatusesToUi(QList< Status > & statusList, QBoxLayout * layoutToAddStatuses, QList<StatusWidget*> *list, Backend::TimeLineType type)
{
	kDebug();
	QList<Status>::const_iterator it = statusList.constBegin();
	QList<Status>::const_iterator endIt = statusList.constEnd();
	
	for(;it!=endIt; ++it){
		if(!isStartMode && (type != Backend::HomeTimeLine || it->replyToUserScreenName!=Settings::username())){
			emit sigNotify(it->user.screenName, it->content,
									 mediaMan->getImageLocalPathIfExist(it->user.profileImageUrl));
		}
		StatusWidget *wt = new StatusWidget(this);
		wt->setAttribute(Qt::WA_DeleteOnClose);
		wt->setCurrentStatus(*it);
		emit sigSetUserImage(wt);
		connect(wt, SIGNAL(sigReply(QString&, uint)), this, SLOT(prepareReply(QString&, uint)));
		connect(wt, SIGNAL(sigFavorite(uint, bool)), twitter, SLOT(requestFavorited(uint, bool)));
		connect(wt, SIGNAL(sigDestroy(uint)), this, SLOT(requestDestroy(uint)));
		list->append(wt);
		layoutToAddStatuses->insertWidget(0, wt);
		if(!isStartMode){
			wt->setUnread();
			listUnreadStatuses.append(wt);
		}
	}
	uint latestId = statusList.last().statusId;
	if(latestId > Settings::latestStatusId()){
		kDebug()<<"Latest sets to: "<<latestId;
		Settings::setLatestStatusId(latestId);
	}
	if(!isStartMode)
		checkUnreadStatuses(statusList.count());
	updateStatusList(list);
}

void MainWindow::setDefaultDirection()
{
// 	this->setLayoutDirection((Qt::LayoutDirection)Settings::direction());
	ui.tabs->widget(0)->setLayoutDirection((Qt::LayoutDirection)Settings::direction());
	ui.tabs->widget(1)->setLayoutDirection((Qt::LayoutDirection)Settings::direction());
// 	txtNewStatus->document()->firstBlock()->
// 	inputLayout->setLayoutDirection((Qt::LayoutDirection)Settings::direction());
	txtNewStatus->setDefaultDirection((Qt::LayoutDirection)Settings::direction());
}

void MainWindow::error(QString & errMsg)
{
	emit sigNotify(i18n("Transaction faild"), errMsg, APPNAME);
}

void MainWindow::postStatus(QString & status)
{
	kDebug();
	//TODO will check for urls!
	if(status.size()>MAX_STATUS_SIZE && status.indexOf("http://")==-1 ){
		QString err = i18n("Status text size is more than server limit size.");
		error(err);
		return;
	}
	statusBar()->showMessage(i18n("Posting New status..."));
	txtNewStatus->setEnabled(false);
	twitter->postNewStatus(status, replyToStatusId);
}

void MainWindow::postingNewStatusDone(bool isError)
{
	if(!isError){
		updateHomeTimeLine();
		txtNewStatus->setText(QString());
		txtNewStatus->clearContentsAndSetDirection((Qt::LayoutDirection)Settings::direction());
		emit sigNotify(i18n("Success!"), i18n("New status posted successfully"), APPNAME);
		replyToStatusId = 0;
	} else {
		error(twitter->latestErrorString());
	}
	txtNewStatus->setEnabled(true);
	if(Settings::hideTwitField())
		toggleTwitFieldVisible();
}

bool MainWindow::saveStatuses(QString fileName, QList<StatusWidget*> &list)
{
	kDebug();
	KConfig statusesBackup(fileName);
	
	int count = list.count();
	for(int i=0; i < count; ++i){
// 		QString str = ;
		KConfigGroup grp(&statusesBackup, QString::number(list[i]->currentStatus().statusId));
		grp.writeEntry("created_at", list[i]->currentStatus().creationDateTime);
		grp.writeEntry("id", list[i]->currentStatus().statusId);
		grp.writeEntry("text", list[i]->currentStatus().content);
		grp.writeEntry("source", list[i]->currentStatus().source);
		grp.writeEntry("truncated", list[i]->currentStatus().isTruncated);
		grp.writeEntry("in_reply_to_status_id", list[i]->currentStatus().replyToStatusId);
		grp.writeEntry("in_reply_to_user_id", list[i]->currentStatus().replyToUserId);
		grp.writeEntry("favorited", list[i]->currentStatus().isFavorited);
		grp.writeEntry("in_reply_to_screen_name", list[i]->currentStatus().replyToUserScreenName);
		grp.writeEntry("userId", list[i]->currentStatus().user.userId);
		grp.writeEntry("screen_name", list[i]->currentStatus().user.screenName);
		grp.writeEntry("profile_image_url", list[i]->currentStatus().user.profileImageUrl);
	}
	statusesBackup.sync();
	return true;
}

QList< Status > MainWindow::loadStatuses(QString fileName)
{
	kDebug();
	KConfig statusesBackup(fileName, KConfig::NoGlobals);
	QList< Status > list;
	QStringList groupList = statusesBackup.groupList();
// 	kDebug()<<groupList;
	int count = groupList.count();
	for(int i=0; i < count; ++i){
		KConfigGroup grp(&statusesBackup, groupList[i]);
		Status st;
		st.creationDateTime = grp.readEntry("created_at", QDateTime::currentDateTime());
		st.statusId = grp.readEntry("id", (uint)0);
		st.content = grp.readEntry("text", QString());
		st.source = grp.readEntry("source", QString());
		st.isTruncated = grp.readEntry("truncated", false);
		st.replyToStatusId = grp.readEntry("in_reply_to_status_id", (uint)0);
		st.replyToUserId = grp.readEntry("in_reply_to_user_id", (uint)0);
		st.isFavorited = grp.readEntry("favorited", false);
		st.replyToUserScreenName = grp.readEntry("in_reply_to_screen_name", QString());
		st.user.userId = grp.readEntry("userId", (uint)0);
		st.user.screenName = grp.readEntry("screen_name", QString());
		st.user.profileImageUrl = grp.readEntry("profile_image_url", QString());
		list.append(st);
	}
	return list;
}

void MainWindow::setUserImage(StatusWidget * widget)
{
	QString imgPath = mediaMan->getImageLocalPathDownloadIfNotExist(widget->currentStatus().user.screenName,
			 widget->currentStatus().user.profileImageUrl, this);
	widget->setUserImage(imgPath);
}

void MainWindow::prepareReply(QString &userName, uint statusId)
{
	kDebug();
	QString current = txtNewStatus->toPlainText();
	txtNewStatus->setText("@"+userName + " " + current);
	replyToStatusId = statusId;
	txtNewStatus->setDefaultDirection((Qt::LayoutDirection)Settings::direction());
	if(!ui.inputFrame->isVisible())
		ui.inputFrame->show();
}

void MainWindow::keyPressEvent(QKeyEvent * e)
{
	if(e->key() == Qt::Key_Escape){
		if(txtNewStatus->isEnabled()){
// 			this->close();
		} else {
			twitter->abortPostNewStatus();
		}
	}
}

void MainWindow::quitApp()
{
	saveStatuses("choqokHomeStatusListrc", listHomeStatus);
	saveStatuses("choqokReplyStatusListrc", listReplyStatus);
	deleteLater();
}

void MainWindow::abortPostNewStatus()
{
	kDebug();
	twitter->abortPostNewStatus();
}

void MainWindow::toggleTwitFieldVisible()
{
	if(ui.inputFrame->isVisible()){
		ui.inputFrame->hide();
		ui.toggleArrow->setArrowType(Qt::LeftArrow);
	}
	else {
		ui.inputFrame->show();
		ui.toggleArrow->setArrowType(Qt::DownArrow);
		txtNewStatus->setFocus(Qt::OtherFocusReason);
	}
}

void MainWindow::updateStatusList(QList<StatusWidget*> *list)
{
	kDebug();
	int toBeDelete = list->count() - Settings::countOfStatusesOnMain();
	if(toBeDelete > 0){
		for(int i =0; i < toBeDelete; ++i){
			StatusWidget* wt = list->first();
			list->removeFirst();
			wt->close();
		}
	}
}

void MainWindow::reloadTimeLineLists()
{
	kDebug();
	clearTimeLineList(&listHomeStatus);
	clearTimeLineList(&listReplyStatus);
	Settings::setLatestStatusId(0);
	updateTimeLines();
	
}

void MainWindow::clearTimeLineList(QList< StatusWidget * > * list)
{
	kDebug();
	int count = list->count();
	for(int i =0; i < count; ++i){
		StatusWidget* wt = list->first();
		list->removeFirst();
		wt->close();
	}
}

void MainWindow::loadConfigurations()
{
	kDebug();
	setDefaultDirection();
	timelineTimer->setInterval(Settings::updateInterval()*60000);
	if(Settings::hideTwitField()){
		ui.inputFrame->hide();
		ui.toggleArrow->setArrowType(Qt::LeftArrow);
	} else{
		ui.toggleArrow->setArrowType(Qt::DownArrow);
		ui.inputFrame->show();
		txtNewStatus->setFocus(Qt::OtherFocusReason);
	}
	
	isStartMode = true;
	QList< Status > lstHome = loadStatuses("choqokHomeStatusListrc");
	if(lstHome.count()>0)
		addNewStatusesToUi(lstHome, ui.homeLayout, &listHomeStatus);
	QList< Status > lstReply = loadStatuses("choqokReplyStatusListrc");
	if(lstReply.count()>0)
		addNewStatusesToUi(lstReply, ui.replyLayout, &listReplyStatus, Backend::ReplyTimeLine);
	
	
	updateTimeLines();
}

void MainWindow::checkUnreadStatuses(int numOfNewStatusesReciened)
{
	kDebug();
	if(this->isVisible()){
		unreadStatusCount = 0;
	} else {
		unreadStatusCount += numOfNewStatusesReciened;
		
	}
	emit sigSetUnread(unreadStatusCount);
}

void MainWindow::setUnreadStatusesToReadState()
{
	///FIXME Bug on changing account!
	kDebug();
	ui.tabs->setTabText(0, i18n("Home"));
	ui.tabs->setTabText(1, i18n("Reply"));
	int count = listUnreadStatuses.count();
	for(int i=0;i<count; ++i){
		listUnreadStatuses[i]->setRead();
	}
	listUnreadStatuses.clear();
	unreadStatusCount = unreadStatusInReply = unreadStatusInHome = 0;
	emit sigSetUnread(unreadStatusCount);
}

bool MainWindow::queryClose()
{
	kDebug();
	setUnreadStatusesToReadState();
	return true;
}

void MainWindow::requestFavoritedDone(bool isError)
{
	kDebug()<<"is Error: "<<isError;
	notify("Done!");
}

void MainWindow::requestDestroyDone(bool isError)
{
	kDebug()<<"is Error: "<<isError;
	notify("Done!");
	toBeDestroied->close();
}

void MainWindow::requestDestroy(uint statusId)
{
	if(KMessageBox::warningYesNo(this, i18n("Are you sure of destroying this status?")) == KMessageBox::Yes){
		twitter->requestDestroy(statusId);
		toBeDestroied = qobject_cast<StatusWidget*>(sender());
		setUnreadStatusesToReadState();
	}
}

#include "mainwindow.moc"
