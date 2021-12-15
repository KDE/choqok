/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BEHAVIORCONFIG_H
#define BEHAVIORCONFIG_H

#include <KCModule>

class BehaviorConfig : public KCModule
{
    Q_OBJECT

public:
    BehaviorConfig(QWidget *parent, const QVariantList &args) ;
    ~BehaviorConfig();

    virtual void save() override;
    virtual void load() override;

private:
    class Private;
    Private *d;
};

#endif
