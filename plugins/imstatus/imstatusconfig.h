/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IMSTATUSCONFIG_H
#define IMSTATUSCONFIG_H

#include <KCModule>

#include "ui_imstatusprefs.h"

class IMStatusConfig : public KCModule
{
    Q_OBJECT
public:
    IMStatusConfig(QWidget *parent, const QVariantList &args);
    ~IMStatusConfig();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();
private:
    Ui_IMStatusPrefsBase ui;
    QStringList imList;
};

#endif // IMSTATUSCONFIG_H
