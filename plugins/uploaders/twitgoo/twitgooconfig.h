/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITGOOCONFIG_H
#define TWITGOOCONFIG_H

#include <KCModule>

#include "ui_twitgooprefs.h"

class TwitgooConfig : public KCModule
{
    Q_OBJECT
public:
    TwitgooConfig(QWidget *parent, const QVariantList &);
    ~TwitgooConfig();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();

private:
    Ui_TwitgooPrefsBase ui;
};

#endif // TWITGOOCONFIG_H
