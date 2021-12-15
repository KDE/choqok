/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKPLUGINCONFIG_H
#define CHOQOKPLUGINCONFIG_H

#include <KCModule>

class KPluginSelector;

/**
 * Plugin selector. See KPluginSelector in kdelibs for documentation.
 *
 * @author
 */
class ChoqokPluginConfig : public KCModule
{
    Q_OBJECT

public:
    ChoqokPluginConfig(QWidget *parent, const QVariantList &args);
    ~ChoqokPluginConfig();

public Q_SLOTS:
    virtual void load() override;
    virtual void save() override;

    virtual void defaults() override;
    void reparseConfiguration(const QByteArray &conf);
private:
    KPluginSelector *m_pluginSelector;
};

#endif

