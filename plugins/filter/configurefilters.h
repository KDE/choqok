/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CONFIGUREFILTERS_H
#define CONFIGUREFILTERS_H

#include <QDialog>

#include "ui_filterprefs.h"

class Filter;
class ConfigureFilters : public QDialog
{
    Q_OBJECT
public:
    ConfigureFilters(QWidget *parent);
    ~ConfigureFilters();

protected Q_SLOTS:
    virtual void accept() override;
    void slotAddFilter();
    void slotEditFilter();
    void slotRemoveFilter();
    void addNewFilter(Filter *filter);
    void slotUpdateFilter(Filter *filter);
    void slotHideRepliesNotRelatedToMeToggled(bool enabled);

private:
    void reloadFiltersTable();
    void saveFiltersTable();
    Ui_mFilteringCtl ui;
};

#endif // CONFIGUREFILTERS_H
