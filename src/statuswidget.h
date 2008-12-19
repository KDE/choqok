//
// C++ Interface: statuswidget
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QFrame>
#include <QTimer>
#include <ui_status_base.h>
#include "datacontainers.h"
#define UPDATEINTERVAL 2*60000
#define COLOROFFSET 15
/**
Status Widget

	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class StatusWidget : public QFrame, public Ui_StatusBase
{
Q_OBJECT
public:
    StatusWidget(QWidget *parent=0);

    ~StatusWidget();
	
	static QString formatDateTime(const QDateTime &time);
	
	Status currentStatus() const;
	void setCurrentStatus(const Status newStatus);
	void setUserImage(const QString &imgPath);
	void setUnread();
	void setRead();
	void updateFavoriteUi();
	
signals:
	void sigReply(QString &userName, uint statusId);
	void sigDestroy(uint statusId);
	void sigFavorite(uint statusId, bool isFavorite);
	
protected slots:
	void setFavorite(bool isFavorite);
	void requestReply();
	void requestDestroy();
	void updateSign();
	
private:
	QString prepareStatus(const QString &text, const int &replyStatusId);
	void setUiStyle();
	QString generateSign();
	void updateUi();
	QTimer timer;
	Status mCurrentStatus;

};

#endif
