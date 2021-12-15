/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UNTINYCONFIG_H
#define UNTINYCONFIG_H

#include <kcmodule.h>
#include "ui_untinyprefs.h"

class UnTinyConfig : public KCModule
{
    Q_OBJECT
public:
    UnTinyConfig(QWidget* parent, const QVariantList& args);
    ~UnTinyConfig();

    virtual void save();
    virtual void load();
    virtual void defaults();

protected Q_SLOTS:
    void emitChanged();
private:
    Ui_UnTinyPrefsBase ui;
};

#endif // UNTINYCONFIG_H
