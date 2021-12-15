/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TINYARRO_WS_H
#define TINYARRO_WS_H

#include <QVariantList>

#include <KCModule>

#include "ui_tinyarro_ws_prefs.h"

class Tinyarro_ws_Config : public KCModule
{
    Q_OBJECT
public:
    Tinyarro_ws_Config(QWidget *parent, const QVariantList &);
    ~Tinyarro_ws_Config();

    virtual void save() override;
    virtual void load() override;
protected:
    QMap<QString, QString> hostList;
protected Q_SLOTS:
    void emitChanged();
private:
    Ui_Tinyarro_ws_PrefsBase ui;
};

#endif // TINYARRO_WS_H
