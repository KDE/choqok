/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef NOTIFYCONFIG_H
#define NOTIFYCONFIG_H

#include <QMap>
#include <QPointer>

#include <KCModule>

#include "ui_notifyprefs.h"

class DummyNotification;

class NotifySettings;
class NotifyConfig : public KCModule
{
    Q_OBJECT
public:
    NotifyConfig(QWidget *parent, const QVariantList &args);
    ~NotifyConfig();

    virtual void save() override;
    virtual void load() override;

protected Q_SLOTS:
    void updateTimelinesList();
    void timelineSelectionChanged();
    void emitChanged();
    void slotAdjustNotificationPosition();
    void slotNewPositionSelected(QPoint);

private:
    QStringList langs;
    Ui_NotifyPrefsBase ui;
    QMap<QString, QStringList> accounts;
    NotifySettings *settings;
    QPointer<DummyNotification> dummy;
};

#endif // NOTIFYCONFIG_H
