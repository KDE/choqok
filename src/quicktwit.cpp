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
#include <KComboBox>
#include "statustextedit.h"
#include "backend.h"
#include "datacontainers.h"
#include "mainwindow.h"
#include "constants.h"
#include "settings.h"
#include "accountmanager.h"

QuickTwit::QuickTwit(QWidget* parent): KDialog(parent)
{
	kDebug();
    QWidget *wdg = new QWidget(this);
    ui.setupUi(wdg);
    
	txtStatus = new StatusTextEdit(this);
    
    loadAccounts();
    
//     txtStatus->setDefaultDirection(accountsList[ui.comboAccounts->currentIndex()].direction);
    ui.layout->addWidget(txtStatus);
	this->setMainWidget(wdg);
	this->resize(280, 140);
	txtStatus->setFocus(Qt::OtherFocusReason);
	
	this->setCaption(i18n("Quick Tweet"));
	ui.lblCounter->setText(QString::number(MAX_STATUS_SIZE));
	
	connect(txtStatus, SIGNAL(returnPressed(QString&)), this, SLOT(slotPostNewStatus(QString&)));
	connect(txtStatus, SIGNAL(charsLeft(int)), this, SLOT(checkNewStatusCharactersCount(int)));
	connect(this, SIGNAL(accepted()), this, SLOT(sltAccepted()));
    connect(ui.checkAll, SIGNAL(toggled(bool)), this, SLOT(checkAll(bool)));
    connect(AccountManager::self(), SIGNAL(accountAdded(const Account&)), this, SLOT(addAccount(const Account&)));
    connect(AccountManager::self(), SIGNAL(accountRemoved(const QString&)), this, SLOT(removeAccount(const QString&)));
}


QuickTwit::~QuickTwit()
{
	kDebug();
}

void QuickTwit::showFocusedOnNewStatusField()
{
    txtStatus->setFocus( Qt::OtherFocusReason );
    this->show();
}

void QuickTwit::checkNewStatusCharactersCount(int numOfChars)
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

void QuickTwit::slotPostNewStatusDone(bool isError)
{
	kDebug();
	if(isError){
        Backend * b = qobject_cast<Backend *>(sender());
        QString name(APPNAME);
        emit sigNotify(i18n("Failed!"), i18n("Posting new status failed. %1", b->latestErrorString()), name);
    }else{
        txtStatus->clearContentsAndSetDirection(accountsList[ui.comboAccounts->currentIndex()].direction);
		QString name(APPNAME);
		emit sigNotify(i18n("Success!"), i18n("New status posted successfully"), name);
	}
	this->close();
    sender()->deleteLater();
}

void QuickTwit::slotPostNewStatus(QString & newStatus)
{
    kDebug();
    if(ui.checkAll->isChecked()){
        int count = accountsList.count();
        for(int i=0;i < count; ++i){
            Backend *twitter = new Backend(&accountsList[i] , this);
            connect(twitter, SIGNAL(sigPostNewStatusDone(bool)), this, SLOT(slotPostNewStatusDone(bool)));
            twitter->postNewStatus(newStatus);
        }
    } else {
        Backend *twitter = new Backend(&accountsList[ui.comboAccounts->currentIndex()] , this);
        connect(twitter, SIGNAL(sigPostNewStatusDone(bool)), this, SLOT(slotPostNewStatusDone(bool)));
        twitter->postNewStatus(newStatus);
    }
	this->hide();
}

void QuickTwit::sltAccepted()
{
	kDebug();
	QString txt = txtStatus->toPlainText();
	slotPostNewStatus(txt);
}

void QuickTwit::loadAccounts()
{
    kDebug();
    QList<Account> ac = AccountManager::self()->accounts();
    QListIterator<Account> it(ac);
    while(it.hasNext()){
        Account current = it.next();
        accountsList.append(current);
        ui.comboAccounts->addItem(current.alias);
    }
    if(ac.count()>0){
        txtStatus->setEnabled(true);
    } else {
        txtStatus->setEnabled(false);
    }
}

void QuickTwit::addAccount(const Account & account)
{
    kDebug();
    accountsList.append(account);
    ui.comboAccounts->addItem(account.alias);
}

void QuickTwit::removeAccount(const QString & alias)
{
    kDebug();
    int count = accountsList.count();
    for(int i=0; i<count; ++i){
        if( accountsList[i].alias == alias ){
            accountsList.removeAt(i);
            ui.comboAccounts->removeItem(i);
            kDebug()<<"Found!";
            return;
        }
    }
}

void QuickTwit::checkAll(bool isAll)
{
    ui.comboAccounts->setEnabled( !isAll );
}

#include "quicktwit.moc"
