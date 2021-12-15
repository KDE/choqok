/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2016 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FRIENDICAMICROBLOGPLUGIN_H
#define FRIENDICAMICROBLOGPLUGIN_H

#include "gnusocialapimicroblog.h"

class ChoqokEditAccountWidget;

class FriendicaMicroBlog : public GNUSocialApiMicroBlog
{
    Q_OBJECT
public:
    FriendicaMicroBlog(QObject *parent, const QVariantList &args);
    ~FriendicaMicroBlog();

    virtual ChoqokEditAccountWidget *createEditAccountWidget(Choqok::Account *account, QWidget *parent) override;
    virtual QUrl profileUrl(Choqok::Account *account, const QString &username) const override;

};

#endif
