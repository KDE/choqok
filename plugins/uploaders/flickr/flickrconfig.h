/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#ifndef FLICKRCONFIG_H
#define FLICKRCONFIG_H

#include <KCModule>

#include "ui_flickrprefs.h"

class FlickrConfig : public KCModule
{
    Q_OBJECT
public:
    FlickrConfig(QWidget *parent, const QVariantList &);
    ~FlickrConfig();

    virtual void save();
    virtual void load();

    void getFrob();
    QByteArray createSign(QByteArray);

protected Q_SLOTS:
    void emitChanged();
    void slotAuthButton_clicked();
    void setAuthenticated(bool);
    void getToken();

protected:
    bool isAuthenticated;

    QString m_frob;
    QString m_token;
    QString m_nsid;
    QString m_username;
    QString m_fullname;
private:
    Ui_FlickrPrefsBase ui;
    QWidget *wd;
};

#endif // FLICKRCONFIG_H
