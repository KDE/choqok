/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IS_GD_CONFIG_H
#define IS_GD_CONFIG_H

#include <KCModule>

#include "ui_is_gd_prefs.h"

class Is_gd_Config : public KCModule
{
    Q_OBJECT
public:
    Is_gd_Config(QWidget *parent, const QVariantList &);
    ~Is_gd_Config();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();

private:
    Ui_Is_gd_Prefs ui;

};

#endif // IS_GD_CONFIG_H
