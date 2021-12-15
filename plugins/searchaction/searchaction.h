/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SEARCHACTION_H
#define SEARCHACTION_H

#include "plugin.h"

class SearchAction : public Choqok::Plugin
{
    Q_OBJECT
public:
    SearchAction(QObject *parent, const QList< QVariant > &args);
    ~SearchAction();

protected Q_SLOTS:
    void slotSearch();
};

#endif // SEARCHACTION_H
