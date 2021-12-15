/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FLICKRCONFIG_H
#define FLICKRCONFIG_H

#include <QUrlQuery>

#include <KCModule>

#include "ui_flickrprefs.h"

class FlickrConfig : public KCModule
{
    Q_OBJECT
public:
    FlickrConfig(QWidget *parent, const QVariantList &);
    ~FlickrConfig();

    virtual void save() override;
    virtual void load() override;

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
