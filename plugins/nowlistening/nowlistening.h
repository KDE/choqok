/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010-2011 Ramin Gomari <ramin.gomari@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef NOWLISTENING_H
#define NOWLISTENING_H

#include "plugin.h"

/**
Now Listening

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
@author Ramin Gomari \<ramin.gomari@gmail.com\>
*/
class NowListening : public Choqok::Plugin
{
    Q_OBJECT
public:
    NowListening(QObject *parent, const QList< QVariant > &args);
    ~NowListening();

protected Q_SLOTS:
    void slotPrepareNowListening();

};

#endif //NOWLISTENING_H
