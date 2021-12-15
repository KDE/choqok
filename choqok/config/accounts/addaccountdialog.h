/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ADDACCOUNTDIALOG_H
#define ADDACCOUNTDIALOG_H

#include <QDialog>

class ChoqokEditAccountWidget;
namespace Choqok
{
class Account;
}

class AddAccountDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddAccountDialog(ChoqokEditAccountWidget *addWidget, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~AddAccountDialog();

protected Q_SLOTS:
    virtual void accept() override;

private:
    ChoqokEditAccountWidget *widget;
};

#endif // ADDACCOUNTDIALOG_H
