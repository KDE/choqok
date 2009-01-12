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
#include "statuswidget.h"
#include "settings.h"
#include "mediamanager.h"
#include <knotification.h>
#include <QProcess>

StatusWidget::StatusWidget(const Account *account, QWidget *parent)
 : QFrame(parent)
{
	setupUi(this);
	mIsReaded = true;
	timer.start(UPDATEINTERVAL);
	mCurrentAccount = account;
	btnFavorite->setIcon(KIcon("rating"));
	btnReply->setIcon(KIcon("edit-undo"));
	btnRemove->setIcon(KIcon("edit-delete"));
	
	this->setMaximumHeight(110);
	
	connect(btnReply, SIGNAL(clicked(bool)), this, SLOT(requestReply()));
	connect(&timer, SIGNAL(timeout()), this, SLOT(updateSign()));
	connect(btnFavorite, SIGNAL(clicked(bool)), this, SLOT(setFavorite(bool)));
	connect(btnRemove, SIGNAL(clicked(bool)), this, SLOT(requestDestroy()));
}

StatusWidget::~StatusWidget()
{
}

void StatusWidget::setFavorite(bool isFavorite)
{
	emit sigFavorite(mCurrentStatus.statusId, isFavorite);
}

Status StatusWidget::currentStatus() const
{
	return mCurrentStatus;
}

void StatusWidget::setCurrentStatus(const Status newStatus)
{
	mCurrentStatus = newStatus;
	updateUi();
}

void StatusWidget::updateUi()
{
// 	kDebug()<<"ScreenName: "<<mCurrentStatus.user.screenName<<"Current: "<<mCurrentStatus.user.userId<<" Settings: "<<Settings::currentUserId();
	if(mCurrentStatus.user.screenName == mCurrentAccount->username){
		btnReply->setVisible(false);
	} else {
		btnRemove->setVisible(false);
	}
	lblSign->setHtml(generateSign());
	lblStatus->setHtml(prepareStatus(mCurrentStatus.content, mCurrentStatus.replyToStatusId));
	lblImage->setToolTip(mCurrentStatus.user.name);
	setUiStyle();
	updateFavoriteUi();
	setUserImage();
}

QString StatusWidget::formatDateTime(const QDateTime &time) 
{
	int seconds = time.secsTo(QDateTime::currentDateTime());
	if (seconds <= 15) return i18n("Just now");
	if (seconds <= 45) return i18np("about 1 second ago", "about %1 seconds ago", seconds);
	int minutes = (seconds - 45 + 59) / 60;
	if (minutes <= 45) return i18np("about 1 minute ago", "about %1 minutes ago", minutes);
	int hours = (seconds - 45 * 60 + 3599) / 3600;
	if (hours <= 18) return i18np("about 1 hour ago", "about %1 hours ago", hours);
	int days = (seconds - 18 * 3600 + 24 * 3600 - 1) / (24 * 3600);
	return i18np("about 1 day ago", "about %1 days ago", days);
}

void StatusWidget::setUserImage(const QString & imgPath)
{
	lblImage->setPixmap(QPixmap(imgPath));
}

void StatusWidget::requestReply()
{
	kDebug();
	emit sigReply(mCurrentStatus.user.screenName, mCurrentStatus.statusId);
}

QString StatusWidget::generateSign()
{
    QString sign;
    if(mCurrentAccount->serviceName.toLower() == QString(IDENTICA_SERVICE_TEXT).toLower()){
        sign = "<b><a href=\"http://identi.ca/"+mCurrentStatus.user.screenName+"\" title=\""+
                mCurrentStatus.user.description + "\">" + mCurrentStatus.user.screenName+"</a> - </b> ";
        sign += "<a href=\"http://identi.ca/notice/" + QString::number(mCurrentStatus.statusId) + "\" title=\"" + 
                mCurrentStatus.creationDateTime.toString()+"\">" + formatDateTime(mCurrentStatus.creationDateTime) + "</a> - ";
        sign += mCurrentStatus.source;
    } else {
        sign = "<b><a href=\"http://twitter.com/"+mCurrentStatus.user.screenName+"\" title=\""+
                mCurrentStatus.user.description + "\">" + mCurrentStatus.user.screenName+"</a> - </b> ";
        sign += "<a href=\"http://twitter.com/" + mCurrentStatus.user.screenName + "/statuses/" +
                QString::number(mCurrentStatus.statusId) + "\" title=\"" + 
                mCurrentStatus.creationDateTime.toString()+"\">" + formatDateTime(mCurrentStatus.creationDateTime) + "</a> - ";
        sign += mCurrentStatus.source;
    }
	return sign;
}

void StatusWidget::updateSign()
{
	lblSign->setText(generateSign());
}

void StatusWidget::requestDestroy()
{
	emit sigDestroy(mCurrentStatus.statusId);
}

QString StatusWidget::prepareStatus(const QString &text, const int &replyStatusId)
{
	QString s = text;
	int i = 0, j = 0;
	///TODO Adding smile support!
/*	if(Settings::isSmilysEnabled()){
		while((j = s.indexOf(':', i)) != -1){
			if(s[j+1]==')' && s[j+2]==')')
				;
			else
				switch(s[j+1]){
					case 'D':
						break;
					case ')':
						break;
					case '(':
						break;
					case 'o':
					case 'O':
						break;
					case '*':
					case 'x':
						break;
					case '|':
						break;
					case '/':
						break;
						
				};
		}
	}*/
	
	s.replace(" www.", " http://www.");
	if (s.startsWith("www.")) s = "http://" + s;
	QString t="";
	i = j = 0;
	while ((j = s.indexOf("http://", i)) != -1) {
		t += s.mid(i, j - i);
		int k = s.indexOf(' ', j);
		if (k == -1) k = s.length();
		QString url = s.mid(j, k - j);
		t += "<a href='" + url + "'>" + url + "</a>";
		i = k;
	}
	t += s.mid(i);
	if (replyStatusId && (t[0] == '@')) {
		s = t;
		int i = 1;
		while ((i < s.length()) && (QChar(s[i]).isLetterOrNumber() || (s[i] == '_'))) ++i;
		QString username = s.mid(1, i - 1);
        QString statusUrl;
        if(mCurrentAccount->serviceName.toLower() == QString(IDENTICA_SERVICE_TEXT).toLower()){
            statusUrl = "http://identi.ca/notice/" + QString::number(replyStatusId);
        }else {
            statusUrl = "http://twitter.com/" + username + "/statuses/" + QString::number(replyStatusId);
        }
        t = "@<a href=\"" + statusUrl + "\" title=\""+ statusUrl +"\" >" + username + "</a>" + s.mid(i);
	}
	if(mCurrentAccount->direction==Qt::RightToLeft){
		s = "<div dir='rtl'>";
	} else {
		s = "<div dir='ltr'>";
	}
	s += t;
	s += "</div>";
	return s;
}

void StatusWidget::setUnread(Notify notifyType)
{
	mIsReaded = false;
    QColor backColor;
    QString sheet;
	if(Settings::isCustomUi()){
		backColor = Settings::newStatusBackColor();
        sheet += " color:" + Settings::newStatusForeColor().name() + ';';
	} else {
		backColor = this->palette().window().color();
		backColor.setBlue( backColor.blue()+COLOROFFSET);
		backColor.setGreen( backColor.green()+COLOROFFSET);
		backColor.setRed( backColor.red()+COLOROFFSET);
	}
    sheet += "background-color: rgb(" + QString::number(backColor.red()) + ','
            + QString::number(backColor.green()) + ',' + QString::number(backColor.blue()) + ");";
	this->setStyleSheet(sheet);
	
	if(notifyType == WithNotify){
		QString iconUrl = MediaManager::self()->getImageLocalPathIfExist(mCurrentStatus.user.profileImageUrl);
		QString name = mCurrentStatus.user.screenName;
		QString msg = mCurrentStatus.content;
		if(Settings::notifyType() == 1){
			KNotification *notify=new KNotification("new-status-arrived", parentWidget());
			notify->setText( QString("<qt><b>" + name + ":</b><br/>" + msg + "</qt>" ) );
			notify->setPixmap(QPixmap(iconUrl));
			notify->setFlags(KNotification::RaiseWidgetOnActivation | KNotification::Persistent);
			notify->setActions( i18n("Reply").split(',') );
			connect(notify,SIGNAL(action1Activated()), this , SLOT(requestReply()) );
			notify->sendEvent();
			QTimer::singleShot(Settings::notifyInterval()*1000, notify, SLOT(close()));
		} else if(Settings::notifyType() == 2){
			QString libnotifyCmd = QString("notify-send -t ") + QString::number(Settings::notifyInterval()*1000) + QString(" -u low -i "+ iconUrl +" \"") + name + QString("\" \"") + msg + QString("\"");
			QProcess::execute(libnotifyCmd);
		}
	}
}

void StatusWidget::setRead()
{
	mIsReaded = true;
    QColor backColor;
    QString sheet;
	if(Settings::isCustomUi()){
        backColor = Settings::defaultBackColor();
        sheet += " color:" + Settings::defaultForeColor().name() + ';';
	} else {
		backColor = this->palette().window().color();
		backColor.setBlue( backColor.blue()-COLOROFFSET);
		backColor.setGreen( backColor.green()-COLOROFFSET);
		backColor.setRed( backColor.red()-COLOROFFSET);
	}
    sheet += "background-color: rgb("+QString::number(backColor.red())+','
            +QString::number(backColor.green())+", "+QString::number(backColor.blue())+");";
	this->setStyleSheet( sheet );
}

void StatusWidget::setUiStyle()
{
	QColor backColor;
    QString sheet;
	if(Settings::isCustomUi()){
        backColor = Settings::defaultBackColor();
        sheet += " color:" + Settings::defaultForeColor().name() + ';';
	} else {
		backColor = this->palette().window().color();
    }
    sheet += "background-color: rgb(" + QString::number(backColor.red()) + ','
            + QString::number(backColor.green()) + ',' + QString::number(backColor.blue()) + ");";
	this->setStyleSheet( sheet );
}

void StatusWidget::updateFavoriteUi()
{
	if(mCurrentStatus.isFavorited){
		btnFavorite->setChecked(true);
	} else {
		btnFavorite->setChecked(false);
	}
}

bool StatusWidget::isReaded()
{
	return mIsReaded;
}

void StatusWidget::setUserImage(const QPixmap * image)
{
	lblImage->setPixmap(*image);
}

void StatusWidget::setUserImage()
{
    connect(MediaManager::self(), SIGNAL(imageFetched(const QString &, const QString &)),
            this, SLOT(userImageLocalPathFetched(const QString&, const QString&)));
    MediaManager::self()->getImageLocalPathDownloadAsyncIfNotExists(mCurrentAccount->serviceName + 
			mCurrentStatus.user.screenName , mCurrentStatus.user.profileImageUrl);
}

void StatusWidget::userImageLocalPathFetched(const QString &remotePath, const QString &localPath)
{
    if(remotePath == mCurrentStatus.user.profileImageUrl){
        lblImage->setPixmap(QPixmap(localPath));
        disconnect(MediaManager::self(), SIGNAL(imageFetched(const QString &, const QString &)),
                this, SLOT(userImageLocalPathFetched(const QString&, const QString&)));
    }
}

#include "statuswidget.moc"
