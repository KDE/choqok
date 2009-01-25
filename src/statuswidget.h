/*
    This file is part of choqoK, the KDE Twitter client

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
#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QFrame>
#include <QTimer>
#include <ui_statuswidget_base.h>
#include "datacontainers.h"
#include "constants.h"
#include "account.h"
#define UPDATEINTERVAL 2*60000
#define COLOROFFSET 20
/**
Status Widget

	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class StatusWidget : public QFrame, public Ui_StatusWidgetBase
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
	void sigReply( const QString &userName, uint statusId);
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
