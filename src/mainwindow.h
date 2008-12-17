/*
 * mainwindow.h
 *
 * Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <kxmlguiwindow.h>

#include "ui_prefs_base.h"
#include "ui_mainwindow_base.h"
#include "ui_accounts_base.h"
#include "datacontainers.h"
#include "backend.h"

#define TIMEOUT 5000
class Backend;
class StatusTextEdit;
class StatusWidget;
class QTimer;
class MediaManagement;
/**
 * This class serves as the main window for choqoK.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    MainWindow();

    /**
     * Default Destructor
     */
	virtual ~MainWindow();
	
// 	static void systemNotify(const QString title, const QString message, QString iconUrl);
public slots:
	void updateTimeLines();
	void updateHomeTimeLine();
	void setUnreadStatusesToReadState();
	void requestFavoritedDone(bool isError);
	void requestDestroyDone(bool isError);
	
protected slots:
    void optionsPreferences();
	void settingsChanged();
	void toggleTwitFieldVisible();
	
	void homeTimeLinesRecived(QList<Status> &statusList);
	void replyTimeLineRecived(QList<Status> &statusList);
	void postingNewStatusDone(bool isError);
	void prepareReply(QString &userName, uint statusId);

	void requestDestroy(uint statusId);
	void notify(const QString &message);
	
	void checkNewStatusCharactersCount(int numOfChars);
	
// 	void postStatus();
	void postStatus(QString &status);
	
	void error(QString &errMsg);
	
	void setUserImage(StatusWidget *widget);
	
	void quitApp();
	
	void abortPostNewStatus();
	
signals:
	void sigSetUserImage(StatusWidget *widget);
	void sigSetUnread(int unread);
	void sigNotify( const QString &title, const QString &message, const QString &iconUrl);
	
protected:
	void keyPressEvent(QKeyEvent * e);
	void checkUnreadStatuses(int numOfNewStatusesReciened);
	bool queryClose();
	
private slots:
	void initObjects();

private:
    void setupActions();
	void setDefaultDirection();
	void addNewStatusesToUi(QList< Status > & statusList, QBoxLayout *layoutToAddStatuses, QList<StatusWidget*> *list,
							 Backend::TimeLineType type = Backend::HomeTimeLine);
// 	void setTxtNewStatusDirection();
// 	QString prepareNewStatus(QString newStatus=QString());

	/**
	 * Will store current first page of statuses on disk.
	 * @param fileName list will be stored on this file.
	 * @param list list of Statuses will be stored.
	 * @return True on success, and false on failer
	 */
	bool saveStatuses(QString fileName, QList<StatusWidget*> &list);
	
	QList< Status > loadStatuses(QString fileName);
	
	void updateStatusList(QList<StatusWidget*> *list);
	
	void reloadTimeLineLists();
	void clearTimeLineList(QList<StatusWidget*> *list);
	
	void loadConfigurations();

private:
	QTimer *timelineTimer;
	Ui::MainWindow_base ui;
    Ui::prefs_base ui_prefs_base;
	Ui::accounts_base ui_accounts_base;
	QWidget *mainWidget;
	Backend *twitter;
	StatusTextEdit *txtNewStatus;
	QList<StatusWidget*> listHomeStatus;
	QList<StatusWidget*> listReplyStatus;
	QList<StatusWidget*> listUnreadStatuses;
	MediaManagement *mediaMan;
	uint replyToStatusId;
	QString currentUsername;// used for undresanding of username changes!
	bool isStartMode;//used for Notify, if true: notify will not send for any or all new twits, if false will send.
	
	int unreadStatusCount;
	short unreadStatusInHome;
	short unreadStatusInReply;
	
	StatusWidget *toBeDestroied;
};

#endif 
