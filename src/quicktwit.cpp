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

#include "quicktwit.h"
#include <QKeyEvent>
#include "statustextedit.h"
#include "backend.h"
#include "mainwindow.h"
#include "constants.h"
#include "settings.h"

QuickTwit::QuickTwit(QWidget* parent): KDialog(parent)
{
	kDebug();
	txtStatus = new StatusTextEdit(this);
	txtStatus->setDefaultDirection((Qt::LayoutDirection)Settings::direction());
	this->setMainWidget(txtStatus);
	this->resize(280, 120);
	txtStatus->setFocus(Qt::OtherFocusReason);
	
	this->setCaption(i18n("What are you doing?"));
	setButtonText(KDialog::Ok, QString::number(MAX_STATUS_SIZE));
	
	twitter = new Backend(this);
	connect(txtStatus, SIGNAL(returnPressed(QString&)), this, SLOT(slotPostNewStatus(QString&)));
	connect(txtStatus, SIGNAL(charsLeft(int)), this, SLOT(checkNewStatusCharactersCount(int)));
	connect(twitter, SIGNAL(sigPostNewStatusDone(bool)), this, SLOT(slotPostNewStatusDone(bool)));
	connect(this, SIGNAL(accepted()), this, SLOT(sltAccepted()));
}


QuickTwit::~QuickTwit()
{
}

void QuickTwit::showNearMouse()
{
	QPoint cursorPos = QCursor::pos();
	
// 	move();
}

void QuickTwit::checkNewStatusCharactersCount(int numOfChars)
{
	setButtonText(KDialog::Ok, QString::number(numOfChars));
}

void QuickTwit::slotPostNewStatusDone(bool isError)
{
	kDebug();
	if(!isError){
		txtStatus->clearContentsAndSetDirection((Qt::LayoutDirection)Settings::direction());
		emit sigStatusUpdated();
		QString name(APPNAME);
		emit sigNotify(i18n("Success!"), i18n("New status posted successfully"), name);
	}
	this->close();
}

void QuickTwit::slotPostNewStatus(QString & newStatus)
{
	kDebug();
	twitter->postNewStatus(newStatus);
	this->hide();
}

void QuickTwit::sltAccepted()
{
	kDebug();
	QString txt = txtStatus->toPlainText();
	slotPostNewStatus(txt);
}

#include "quicktwit.moc"
