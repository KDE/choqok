/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TRANSLATORCONFIG_H
#define TRANSLATORCONFIG_H

#include <kcmodule.h>
#include "ui_translatorprefs.h"

class TranslatorConfig : public KCModule
{
    Q_OBJECT
public:
    TranslatorConfig(QWidget* parent, const QVariantList& args);
    ~TranslatorConfig();

    virtual void save();
    virtual void load();
    virtual void defaults();

protected Q_SLOTS:
    void emitChanged();
private:
    QStringList langs;
    Ui_TranslatorPrefsBase ui;
};

#endif // TRANSLATORCONFIG_H
