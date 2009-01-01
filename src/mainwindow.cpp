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
	
	this->setAttribute(Qt::WA_DeleteOnClose, false);

	mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
	ui.homeLayout->setDirection(QBoxLayout::TopToBottom);
	ui.replyLayout->setDirection(QBoxLayout::TopToBottom);
	txtNewStatus = new StatusTextEdit(mainWidget);
	txtNewStatus->setObjectName("txtNewStatus");
	ui.inputFrame->layout()->addWidget(txtNewStatus);
	
	setCentralWidget(mainWidget);
	setupActions();
	statusBar()->show();
	notify("Initializing choqoK UI, please be patient...");
	setupGUI();
	
	connect(ui.toggleArrow, SIGNAL(clicked()), this, SLOT(toggleTwitFieldVisible()));
	connect(txtNewStatus, SIGNAL(charsLeft(int)), this, SLOT(checkNewStatusCharactersCount(int)));
	connect(txtNewStatus, SIGNAL(returnPressed(QString&)), this, SLOT(postStatus(QString&)));
// 	connect(this, SIGNAL(sigSetUserImage(StatusWidget*)), this, SLOT(setUserImage(StatusWidget*)));
	
	QTimer::singleShot( 0, this, SLOT(initObjects()) );
}

void MainWindow::initObjects()
{
	kDebug();
	
	txtNewStatus->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
	txtNewStatus->setMaximumHeight(80);
	txtNewStatus->setFocus(Qt::OtherFocusReason);
	txtNewStatus->setTabChangesFocus ( true );
	
	Settings::setLatestStatusId(0);
	Settings::self()->readConfig();
	twitter = new Backend(this);
	connect(twitter, SIGNAL(homeTimeLineRecived(QList< Status >&)), this, SLOT(homeTimeLinesRecived(QList< Status >&)));
	connect(twitter, SIGNAL(replyTimeLineRecived(QList< Status >&)), this, SLOT(replyTimeLineRecived(QList< Status >&)));
	connect(twitter, SIGNAL(sigPostNewStatusDone(bool)), this, SLOT(postingNewStatusDone(bool)));
	connect(twitter, SIGNAL(sigFavoritedDone(bool)), this, SLOT(requestFavoritedDone(bool)));
	connect(twitter, SIGNAL(sigDestroyDone(bool)), this, SLOT(requestDestroyDone(bool)));
	connect(twitter, SIGNAL(sigError(QString&)), this, SLOT(error(QString&)));
	
	replyToStatusId = unreadStatusCount = unreadStatusInReply = unreadStatusInHome = 0;

	
	timelineTimer = new QTimer(this);
	timelineTimer->setInterval(Settings::updateInterval()*60000);
	timelineTimer->start();
		
// 	mediaMan = new MediaManagement(this);
	
	connect(timelineTimer, SIGNAL(timeout()), this, SLOT(updateTimeLines()));
	
	loadConfigurations();
}

MainWindow::~MainWindow()
{
	kDebug();
	timelineTimer->stop();
	Settings::self()->writeConfig();
}

void MainWindow::setupActions()
{
	KStandardAction::quit(qApp, SLOT(quit()), actionCollection());
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(quitApp()));
	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	KAction *actUpdate = new KAction(KIcon("view-refresh"), i18n("Update timelines"), this);
	actionCollection()->addAction(QLatin1String("update_timeline"), actUpdate);
	actUpdate->setShortcut(Qt::Key_F5);
	actUpdate->setGlobalShortcutAllowed(true);
	KShortcut updateGlobalShortcut(Qt::ControlModifier | Qt::MetaModifier | Qt::Key_F5);
	updateGlobalShortcut.setAlternate(Qt::MetaModifier | Qt::Key_F5);
	actUpdate->setGlobalShortcut(updateGlobalShortcut);
	connect(actUpdate, SIGNAL(triggered( bool )), this, SLOT(updateTimeLines()));
	
	KAction *newTwit = new KAction(this);
	newTwit->setIcon(KIcon("document-new"));
	newTwit->setText(i18n("Quick Tweet"));
	actionCollection()->addAction(QLatin1String("choqok_new_twit"), newTwit);
	newTwit->setShortcut(Qt::ControlModifier | Qt::Key_T);
	newTwit->setGlobalShortcutAllowed(true);
	KShortcut quickTwitGlobalShortcut(Qt::ControlModifier | Qt::MetaModifier | Qt::Key_T);
	quickTwitGlobalShortcut.setAlternate(Qt::MetaModifier | Qt::Key_T);
	newTwit->setGlobalShortcut(quickTwitGlobalShortcut);
	
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
    dialog->addPage(generalSettingsDlg, i18n("General"), "configure");
	
	QWidget *accountsSettingsDlg = new QWidget;
	ui_accounts_base.setupUi(accountsSettingsDlg);
	dialog->addPage(accountsSettingsDlg, i18n("Account"), "user-properties");
	
	QWidget *appearsSettingsDlg = new QWidget;
	ui_appears_base.setupUi(appearsSettingsDlg);
	dialog->addPage(appearsSettingsDlg, i18n("Appearances"), "format-stroke-color");
	
    connect(dialog, SIGNAL(settingsChanged(QString)), this, SLOT(settingsChanged()));
	currentUsername = Settings::username();
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->show();
}

void MainWindow::checkNewStatusCharactersCount(int numOfChars)
{
	if(numOfChars < 0){
		ui.lblCounter->setStyleSheet("QLabel {color: red}");
	} else if(numOfChars < 30){
		ui.lblCounter->setStyleSheet("QLabel {color: rgb(255, 255, 0);}");
	} else {
		ui.lblCounter->setStyleSheet("QLabel {color: green}");
	}
	ui.lblCounter->setText(i18n("%1", numOfChars));
}

void MainWindow::settingsChanged()
{
	kDebug();
	if(Settings::username().isEmpty() || Settings::password().isEmpty()){
		disableApp();
	} else {
		enableApp();
	}
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
	statusBar()->showMessage(i18n("Latest friends timeline received!"), TIMEOUT);
	int count = statusList.count();
	if(count==0){
		kDebug()<<"Status list is empty";
		statusBar()->showMessage(i18n("No new statuses received. The list is up to date."), TIMEOUT);
		return;
	} else {
		int count = statusList.count();
		kDebug()<<count<<" Statuses received.";
		if(!isStartMode){
			unreadStatusInHome+=count;
			ui.tabs->setTabText(0, i18n("Home(%1)", unreadStatusInHome));
		}
		addNewStatusesToUi(statusList, ui.homeLayout, &listHomeStatus);
		ui.homeScroll->verticalScrollBar()->setSliderPosition ( 0 );
	}
}

void MainWindow::replyTimeLineRecived(QList< Status > & statusList)
{
	kDebug();
	statusBar()->showMessage(i18n("Latest replies timeline received!"), TIMEOUT);
	if(statusList.count()==0){
		kDebug()<<"Status list is empty";
		statusBar()->showMessage(i18n("No new statuses received. The list is up to date."), TIMEOUT);
		return;
	}else {
		int count = statusList.count();
		kDebug()<<count<<" Statuses received.";
		if(!isStartMode){
			unreadStatusInReply+=count;
			ui.tabs->setTabText(1, i18n("Reply(%1)", unreadStatusInReply));
		}
		addNewStatusesToUi(statusList, ui.replyLayout, &listReplyStatus, Backend::ReplyTimeLine);
		ui.replyScroll->verticalScrollBar()->setSliderPosition ( 0 );
	}
}

void MainWindow::addNewStatusesToUi(QList< Status > & statusList, QBoxLayout * layoutToAddStatuses,
									 QList<StatusWidget*> *list, Backend::TimeLineType type)
{
	kDebug();
	bool allInOne = Settings::showAllNotifiesInOne();
	QString notifyStr;
	int numOfNewStatuses = statusList.count();
	QList<Status>::const_iterator it = statusList.constBegin();
	QList<Status>::const_iterator endIt = statusList.constEnd();
	for(;it!=endIt; ++it){
		if(it->replyToUserScreenName == Settings::username() && type == Backend::HomeTimeLine){
			continue;
		}
		StatusWidget *wt = new StatusWidget(this);
		wt->setAttribute(Qt::WA_DeleteOnClose);
		wt->setCurrentStatus(*it);
		connect(wt, SIGNAL(sigReply(QString&, uint)), this, SLOT(prepareReply(QString&, uint)));
		connect(wt, SIGNAL(sigFavorite(uint, bool)), twitter, SLOT(requestFavorited(uint, bool)));
		connect(wt, SIGNAL(sigDestroy(uint)), this, SLOT(requestDestroy(uint)));
		list->append(wt);
		layoutToAddStatuses->insertWidget(0, wt);
		if(!isStartMode){
			if(it->user.screenName == Settings::username()){
				--numOfNewStatuses;
				wt->setUnread(StatusWidget::WithoutNotify);
			} else {
				if(allInOne){
					notifyStr += "<b>" + it->user.screenName + " : </b>" + it->content + '\n';
					wt->setUnread(StatusWidget::WithoutNotify);
				} else {
					wt->setUnread(StatusWidget::WithNotify);
				}
			}
			listUnreadStatuses.append(wt);
		}
	}
	uint latestId = statusList.last().statusId;
	if(latestId > Settings::latestStatusId()){
		kDebug()<<"Latest sets to: "<<latestId;
		Settings::setLatestStatusId(latestId);
	}
	if(!isStartMode)
		checkUnreadStatuses(numOfNewStatuses);
	if(!notifyStr.isEmpty()){
		emit sigNotify(i18n("New statuses"), notifyStr, APPNAME);
	}
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
	notify(i18n("Failed, %1",errMsg));
}

void MainWindow::postStatus(QString & status)
{
	kDebug();
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
		QString successMsg = i18n("New status posted successfully");
		emit sigNotify(i18n("Success!"), successMsg, APPNAME);
		notify(successMsg);
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
// 	statusesBackup.deleteGroup("Statuses");
// 	statusesBackup.sync();
// 	KConfigGroup entries(&statusesBackup, "Statuses");
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
		grp.writeEntry("name", list[i]->currentStatus().user.name);
		grp.writeEntry("profile_image_url", list[i]->currentStatus().user.profileImageUrl);
	}
	statusesBackup.sync();
	return true;
}

QList< Status > MainWindow::loadStatuses(QString fileName)
{
	kDebug();
	KConfig statusesBackup(fileName, KConfig::NoGlobals);
// 	KConfigGroup entries(&statusesBackup, "Statuses");
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
		st.user.name = grp.readEntry("name", QString());
		st.user.profileImageUrl = grp.readEntry("profile_image_url", QString());
		list.append(st);
	}
	return list;
}

void MainWindow::prepareReply(QString &userName, uint statusId)
{
	kDebug();
	if(!this->isVisible())
		this->show();
	QString current = txtNewStatus->toPlainText();
	txtNewStatus->setText('@'+userName + ' ' + current);
	replyToStatusId = statusId;
	txtNewStatus->setDefaultDirection((Qt::LayoutDirection)Settings::direction());
	if(!ui.inputFrame->isVisible())
		ui.inputFrame->show();
	txtNewStatus->moveCursor(QTextCursor::End);
}

void MainWindow::keyPressEvent(QKeyEvent * e)
{
	if(e->key() == Qt::Key_Escape){
		if(txtNewStatus->isEnabled()){
			this->hide();
		} else {
			txtNewStatus->setEnabled(true);
			twitter->abortPostNewStatus();
		}
	} else {
		KXmlGuiWindow::keyPressEvent(e);
	}
}

void MainWindow::quitApp()
{
	twitter->quiting();
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
			StatusWidget* wt = list->at(i);
			if(!wt->isReaded())
				break;
			list->removeAt(i);
			--i;
			--toBeDelete;
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
	if(Settings::username().isEmpty() || Settings::password().isEmpty()){
		if(KMessageBox::questionYesNo(this, i18n("There isn't any account configured yet.\nTo use this app, you need a twitter.com account.\
				\nWould you like to add your account now?")) == KMessageBox::Yes){
					optionsPreferences();
				} else {
					disableApp();
				}
	} else {
		updateTimeLines();
	}
}

void MainWindow::checkUnreadStatuses(int numOfNewStatusesReciened)
{
	kDebug();
// 	if(this->isVisible()){
// 		unreadStatusCount = 0;
// 	} else {
		unreadStatusCount += numOfNewStatusesReciened;
// 	}
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

void MainWindow::disableApp()
{
	kDebug();
	txtNewStatus->setEnabled(false);
	timelineTimer->stop();
	actionCollection()->action("update_timeline")->setEnabled(false);
	actionCollection()->action("choqok_new_twit")->setEnabled(false);
}

void MainWindow::enableApp()
{
	kDebug();
	txtNewStatus->setEnabled(true);
	timelineTimer->start();
	actionCollection()->action("update_timeline")->setEnabled(true);
	actionCollection()->action("choqok_new_twit")->setEnabled(true);
}

#include "mainwindow.moc"
