/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITPICCONFIG_H
#define TWITPICCONFIG_H

#include <KCModule>

#include "ui_twitpicprefs.h"

class TwitpicConfig : public KCModule
{
    Q_OBJECT
public:
    TwitpicConfig(QWidget *parent, const QVariantList &);
    ~TwitpicConfig();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void emitChanged();

private:
    Ui_TwitpicPrefsBase ui;
};

#endif // TWITPICCONFIG_H
