/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BEHAVIORCONFIG_SHORTEN_H
#define BEHAVIORCONFIG_SHORTEN_H

#include "ui_behaviorconfig_shorten_base.h"

#include <QWidget>

#include "shortener.h"

class KPluginMetaData;
class KCModuleProxy;
class BehaviorConfig_Shorten: public QWidget, public Ui::BehaviorConfig_ShortenBase
{
    Q_OBJECT
public:
    BehaviorConfig_Shorten(QWidget *parent = nullptr);
    ~BehaviorConfig_Shorten();
    void load();
    void save();

Q_SIGNALS:
    void changed(bool isChanged);

private Q_SLOTS:
    void currentPluginChanged(int index);

protected Q_SLOTS:
    void slotAboutClicked();
    void slotConfigureClicked();

private:
    QMap<QString, KPluginMetaData> availablePlugins;
    Choqok::Shortener *currentShortener;
    QString prevShortener;
};

#endif
