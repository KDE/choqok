/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ADDEDITFILTER_H
#define ADDEDITFILTER_H

#include <QDialog>

#include "filter.h"
#include "ui_addeditfilter_base.h"

class AddEditFilter : public QDialog
{
    Q_OBJECT
public:
    explicit AddEditFilter(QWidget *parent, Filter *filter = nullptr);
    ~AddEditFilter();

Q_SIGNALS:
    void newFilterRegistered(Filter *filter);
    void filterUpdated(Filter *filter);

protected Q_SLOTS:
    void slotFilterActionChanged(int);
    virtual void accept() override;

private:
    void setupFilterFields();
    void setupFilterTypes();
    void setupFilterActions();
    Ui::AddEditFilterBase ui;
    Filter *currentFilter;
};

#endif // ADDEDITFILTER_H
