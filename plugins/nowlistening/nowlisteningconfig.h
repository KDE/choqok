/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef NOWLISTENINGCONFIG_H
#define NOWLISTENINGCONFIG_H

#include <KCModule>

#include "ui_nowlisteningprefs.h"

class NowListeningConfig : public KCModule
{
    Q_OBJECT
public:
    NowListeningConfig(QWidget *parent, const QVariantList &args);
    ~NowListeningConfig();

    virtual void save() override;
    virtual void load() override;
    virtual void defaults() override;

protected Q_SLOTS:
    void emitChanged();
private:
    Ui_NowListeningPrefsBase ui;
};

#endif // NOWLISTENINGCONFIG_H
