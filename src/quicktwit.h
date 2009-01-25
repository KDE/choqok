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
#ifndef QUICKTWIT_H
#define QUICKTWIT_H

#include "datacontainers.h"
#include "ui_quicktwit_base.h"
#include <kdialog.h>
#include "account.h"
class StatusTextEdit;
class Backend;
/**
Widget for Quik twitting

	@author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class QuickTwit : public KDialog
{
	Q_OBJECT
public:
  QuickTwit(QWidget* parent=0);

    ~QuickTwit();
public slots:
    void showFocusedOnNewStatusField();
	void checkNewStatusCharactersCount(int numOfChars);
	void slotPostNewStatus(QString &newStatus);
	void slotPostNewStatusDone(bool isError);
	void sltAccepted();
    void addAccount(const Account &account);
    void removeAccount(const QString &alias);

protected:
    void loadAccounts();
	
signals:
	void sigStatusUpdated( bool isError );
	void sigNotify( const QString &title, const QString &message, const QString &iconUrl);

protected slots:
    void checkAll(bool isAll);
private:
    Ui::quicktwit_base ui;
	StatusTextEdit *txtStatus;
// 	Backend *twitter;
    QList<Account> accountsList;
};

#endif
