/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLUGINS_IMSTATUS_H
#define PLUGINS_IMSTATUS_H

#include "plugin.h"

#include "choqoktypes.h"

class IMStatusPrivate;

/**
  @author Andrey Esin \<gmlastik@gmail.com\>
*/

class IMStatus : public Choqok::Plugin
{
    Q_OBJECT
public:
    IMStatus(QObject *parent, const QList< QVariant > &args);
    ~IMStatus();

public Q_SLOTS:
    void slotIMStatus(Choqok::JobResult res, Choqok::Post *newPost);
    void update();

private:
    IMStatusPrivate *const d;
};

#endif // PLUGINS_IMSTATUS_H
