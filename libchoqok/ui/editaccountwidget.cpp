/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "editaccountwidget.h"

class ChoqokEditAccountWidgetPrivate
{
public:
    Choqok::Account *account;
};

ChoqokEditAccountWidget::ChoqokEditAccountWidget(Choqok::Account *account, QWidget *parent)
    : QWidget(parent), d(new ChoqokEditAccountWidgetPrivate)
{
    d->account = account;
}

ChoqokEditAccountWidget::~ChoqokEditAccountWidget()
{
    delete d;
}

Choqok::Account *ChoqokEditAccountWidget::account() const
{
    return d->account;
}

bool ChoqokEditAccountWidget::validateData()
{
    return true;
}

void ChoqokEditAccountWidget::setAccount(Choqok::Account *account)
{
    delete d->account;
    d->account = account;
}

#include "moc_editaccountwidget.cpp"
