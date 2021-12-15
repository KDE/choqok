/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KONQCHOQOK_H
#define KONQCHOQOK_H

class QDBusInterface;

#include <KParts/Plugin>

class KonqPluginChoqok : public KParts::Plugin
{
    Q_OBJECT
public:
    KonqPluginChoqok(QObject *parent, const QVariantList &);

    virtual ~KonqPluginChoqok();

private Q_SLOTS:
    void slotpostSelectedText();
    void toggleShortening(bool);
    void updateActions();

private:
    QDBusInterface *m_interface;

};

#endif // KONQCHOQOK_H
