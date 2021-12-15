/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef OCSCONFIGUREWIDGET_H
#define OCSCONFIGUREWIDGET_H

#include "editaccountwidget.h"
#include "ui_ocsconfigurebase.h"

class OCSMicroblog;
class OCSAccount;
class OCSConfigureWidget : public ChoqokEditAccountWidget, Ui::OCSConfigureBase
{
    Q_OBJECT
public:
    explicit OCSConfigureWidget(OCSMicroblog *microblog, OCSAccount *account, QWidget *parent);
    ~OCSConfigureWidget();
    virtual bool validateData() override;
    virtual Choqok::Account *apply() override;

protected Q_SLOTS:
    void slotprovidersLoaded();

private:
    OCSAccount *mAccount;
    OCSMicroblog *mMicroblog;
    bool providersLoaded;
};

#endif // OCSCONFIGUREWIDGET_H
