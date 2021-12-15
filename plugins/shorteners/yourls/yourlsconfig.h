/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef YOURLSCONFIG_H
#define YOURLSCONFIG_H

#include <QVariantList>

#include <KCModule>

#include "ui_yourlsprefs.h"

class YourlsConfig : public KCModule
{
    Q_OBJECT
public:
    YourlsConfig(QWidget *parent, const QVariantList &);
    ~YourlsConfig();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();
private:
    Ui_YourlsPrefsBase ui;
};

#endif // YOURLSCONFIG_H
