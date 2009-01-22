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
#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QFrame>
#include <QTimer>
#include <ui_status_base.h>
#include "datacontainers.h"
#include "constants.h"
#include "account.h"
#define UPDATEINTERVAL 2*60000
#define COLOROFFSET 20
/**
Status Widget

	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class StatusWidget : public QFrame, public Ui_StatusBase
{
Q_OBJECT
public:
	enum Notify{ WithNotify=0, WithoutNotify};
    
    explicit StatusWidget(const Account *account, QWidget *parent = 0);

    ~StatusWidget();
	
	static QString formatDateTime(const QDateTime &time);
	
	Status currentStatus() const;
	void setCurrentStatus(const Status newStatus);
	void setUserImage(const QString &imgPath);
	void setUserImage(const QPixmap *image);
	void setUnread(Notify notifyType);
	void setRead();
	void updateFavoriteUi();
	bool isReaded();
	
signals:
	void sigReply(QString &userName, uint statusId);
	void sigDestroy(uint statusId);
	void sigFavorite(uint statusId, bool isFavorite);
	
protected slots:
	void setFavorite(bool isFavorite);
	void requestReply();
	void requestDestroy();
	void updateSign();
    void userImageLocalPathFetched(const QString &remotePath, const QString &localPath);
	
private:
	void setUserImage();
	QString prepareStatus(const QString &text, const int &replyStatusId);
	void setUiStyle();
	QString generateSign();
	void updateUi();
	
	QTimer timer;
	Status mCurrentStatus;
	bool mIsReaded;
    const Account *mCurrentAccount;
};

#endif
