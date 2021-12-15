/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BIT_LY_CONFIG_H
#define BIT_LY_CONFIG_H

#include <QVariantList>
#include <QUrlQuery>

#include <KCModule>

#include "ui_bit_ly_prefs.h"

class Bit_ly_Config : public KCModule
{
    Q_OBJECT
public:
    Bit_ly_Config(QWidget *parent, const QVariantList &);
    ~Bit_ly_Config();

    QStringList domains;
    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();
    void slotValidate();

private:
    Ui_Bit_ly_Prefs ui;

};

#endif // BIT_LY_CONFIG_H
