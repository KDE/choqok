/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPISEARCHDIALOG_H
#define TWITTERAPISEARCHDIALOG_H

#include <QDialog>

#include "twitterapihelper_export.h"

class TwitterApiAccount;
class TWITTERAPIHELPER_EXPORT TwitterApiSearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TwitterApiSearchDialog(TwitterApiAccount *theAccount, QWidget *parent = nullptr);
    ~TwitterApiSearchDialog();

protected:
    void createUi();
    void fillSearchTypes();

protected Q_SLOTS:
    virtual void accept() override;
    void slotSearchTypeChanged(int);

private:
    class Private;
    Private *const d;
};

#endif // TWITTERAPISEARCHDIALOG_H
