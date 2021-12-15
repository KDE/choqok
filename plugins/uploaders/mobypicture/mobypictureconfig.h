/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MOBYPICTURECONFIG_H
#define MOBYPICTURECONFIG_H

#include <KCModule>

#include "ui_mobypictureprefs.h"

class MobypictureConfig : public KCModule
{
    Q_OBJECT
public:
    MobypictureConfig(QWidget *parent, const QVariantList &);
    ~MobypictureConfig();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();

private:
    Ui_MobypicturePrefsBase ui;
};

#endif // MOBYPICTURECONFIG_H
