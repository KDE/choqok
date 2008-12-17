//
// C++ Implementation: quicktwit
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "quicktwit.h"
#include <QKeyEvent>
#include "statustextedit.h"
#include "backend.h"
#include "mainwindow.h"
#include "constants.h"
#include "settings.h"

QuickTwit::QuickTwit(QWidget* parent): KDialog(parent)
{
	txtStatus = new StatusTextEdit(this);
	txtStatus->setDefaultDirection((Qt::LayoutDirection)Settings::direction());
	this->setMainWidget(txtStatus);
	this->resize(280, 120);
	txtStatus->setFocus(Qt::OtherFocusReason);
	
	this->setCaption("What are you doing?");
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
