/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LACONICAMICROBLOGPLUGIN_H
#define LACONICAMICROBLOGPLUGIN_H

#include "gnusocialapimicroblog.h"

class ChoqokEditAccountWidget;

/**
This plugin is to GNU social service.

@Note GNU social was called StatusNet and Laconcia previously, So I just renamed it on UI :D

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class LaconicaMicroBlog : public GNUSocialApiMicroBlog
{
    Q_OBJECT
public:
    LaconicaMicroBlog(QObject *parent, const QVariantList &args);
    ~LaconicaMicroBlog();

    virtual ChoqokEditAccountWidget *createEditAccountWidget(Choqok::Account *account, QWidget *parent) override;
};

#endif
