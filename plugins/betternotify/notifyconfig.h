/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/

#ifndef NOTIFYCONFIG_H
#define NOTIFYCONFIG_H

#include <kcmodule.h>
#include "ui_notifyprefs.h"
#include <QMap>

class NotifySettings;
class NotifyConfig : public KCModule
{
    Q_OBJECT
public:
    NotifyConfig(QWidget* parent, const QVariantList& args);
    ~NotifyConfig();

    virtual void save();
    virtual void load();

protected slots:
    void updateTimelinesList();
    void timelineSelectionChanged();
    void emitChanged();

private:
    QStringList langs;
    Ui_NotifyPrefsBase ui;
    QMap<QString, QStringList> accounts;
    NotifySettings *settings;
};

#endif // NOTIFYCONFIG_H
