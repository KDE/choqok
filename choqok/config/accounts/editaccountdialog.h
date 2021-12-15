/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef EDITACCOUNTDIALOG_H
#define EDITACCOUNTDIALOG_H

#include <QDialog>

class ChoqokEditAccountWidget;

class EditAccountDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditAccountDialog(ChoqokEditAccountWidget *editWidget, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~EditAccountDialog();

protected Q_SLOTS:
    virtual void accept() override;

private:
    ChoqokEditAccountWidget *widget;
};

#endif // EDITACCOUNTDIALOG_H
