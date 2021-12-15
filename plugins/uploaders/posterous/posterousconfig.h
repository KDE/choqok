/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef POSTEROUSCONFIG_H
#define POSTEROUSCONFIG_H

#include <KCModule>

#include "ui_posterousprefs.h"

class PosterousConfig : public KCModule
{
    Q_OBJECT
public:
    PosterousConfig(QWidget *parent, const QVariantList &);
    ~PosterousConfig();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();

private:
    Ui_PosterousPrefsBase ui;
};

#endif // POSTEROUSCONFIG_H
