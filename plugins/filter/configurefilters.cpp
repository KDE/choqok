/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "configurefilters.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPointer>
#include <QPushButton>

#include <KLocalizedString>

#include "addeditfilter.h"
#include "filtersettings.h"

ConfigureFilters::ConfigureFilters(QWidget *parent):
    QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    ui.setupUi(this);

    mainLayout->addLayout(ui.horizontalLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConfigureFilters::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigureFilters::reject);
    mainLayout->addWidget(buttonBox);

    resize(500, 300);

    connect(ui.btnAdd, &QPushButton::clicked, this, &ConfigureFilters::slotAddFilter);
    connect(ui.btnEdit, &QPushButton::clicked, this, &ConfigureFilters::slotEditFilter);
    connect(ui.btnRemove, &QPushButton::clicked, this, &ConfigureFilters::slotRemoveFilter);
    connect(ui.cfg_hideRepliesNotRelatedToMe, &QCheckBox::toggled, this,
            &ConfigureFilters::slotHideRepliesNotRelatedToMeToggled);
    reloadFiltersTable();
}

ConfigureFilters::~ConfigureFilters()
{
}

void ConfigureFilters::accept()
{
    saveFiltersTable();
    QDialog::accept();
}

void ConfigureFilters::reloadFiltersTable()
{
    ui.filters->clearContents();
    QList<Filter *> filters = FilterSettings::self()->filters();
    //qDebug() << filters.count();
    for (Filter *filter: filters) {
        addNewFilter(filter);
    }
    ui.cfg_hideNoneFriendsReplies->setChecked(FilterSettings::hideNoneFriendsReplies());
    ui.cfg_hideRepliesNotRelatedToMe->setChecked(FilterSettings::hideRepliesNotRelatedToMe());
}

void ConfigureFilters::saveFiltersTable()
{
    QList<Filter *> list;
    int count = ui.filters->rowCount();
    for (int i = 0; i < count; ++i) {
        Filter::FilterField field = (Filter::FilterField) ui.filters->item(i, 0)->data(32).toInt();
        Filter::FilterType type = (Filter::FilterType) ui.filters->item(i, 1)->data(32).toInt();
        Filter::FilterAction action = (Filter::FilterAction) ui.filters->item(i, 3)->data(32).toInt();
        QString text = ui.filters->item(i, 2)->text();
        bool dontHideReplies = ui.filters->item(i, 2)->data(32).toBool();
        Filter *f = new Filter(text, field, type, action, dontHideReplies, FilterSettings::self());
        list << f;
    }
    FilterSettings::self()->setFilters(list);
    FilterSettings::setHideNoneFriendsReplies(ui.cfg_hideNoneFriendsReplies->isChecked());
    FilterSettings::setHideRepliesNotRelatedToMe(ui.cfg_hideRepliesNotRelatedToMe->isChecked());
    FilterSettings::self()->writeConfig();
}

void ConfigureFilters::slotAddFilter()
{
    AddEditFilter *f = new AddEditFilter(this);
    connect(f, &AddEditFilter::newFilterRegistered, this, &ConfigureFilters::addNewFilter);
    f->show();
}

void ConfigureFilters::slotEditFilter()
{
    if (ui.filters->selectedItems().count() > 0) {
        int row = ui.filters->currentRow();
        Filter::FilterField field = (Filter::FilterField) ui.filters->item(row, 0)->data(32).toInt();
        Filter::FilterType type = (Filter::FilterType) ui.filters->item(row, 1)->data(32).toInt();
        Filter::FilterAction action = (Filter::FilterAction) ui.filters->item(row, 3)->data(32).toInt();
        bool dontHideReplies = ui.filters->item(row, 2)->data(32).toBool();
        QString text = ui.filters->item(row, 2)->text();
        Filter *f = new Filter(text, field, type, action, dontHideReplies, this);
        QPointer<AddEditFilter> dialog = new AddEditFilter(this, f);
        connect(dialog, &AddEditFilter::filterUpdated, this, &ConfigureFilters::slotUpdateFilter);
        dialog->exec();
    }
}

void ConfigureFilters::slotRemoveFilter()
{
    if (ui.filters->selectedItems().count() > 0) {
        int row = ui.filters->currentRow();
        ui.filters->removeRow(row);
    }
}

void ConfigureFilters::addNewFilter(Filter *filter)
{
    int row = ui.filters->rowCount();
    ui.filters->insertRow(row);
    QTableWidgetItem *item1 = new QTableWidgetItem(FilterSettings::self()->filterFieldName(filter->filterField()));
    item1->setData(32, filter->filterField());
    ui.filters->setItem(row, 0, item1);
    QTableWidgetItem *item2 = new QTableWidgetItem(FilterSettings::self()->filterTypeName(filter->filterType()));
    item2->setData(32, filter->filterType());
    ui.filters->setItem(row, 1, item2);
    QTableWidgetItem *item3 = new QTableWidgetItem(filter->filterText());
    item3->setData(32, filter->dontHideReplies());
    ui.filters->setItem(row, 2, item3);
    QTableWidgetItem *item4 = new QTableWidgetItem(FilterSettings::self()->filterActionName(filter->filterAction()));
    item4->setData(32, filter->filterAction());
    ui.filters->setItem(row, 3, item4);
}

void ConfigureFilters::slotUpdateFilter(Filter *filter)
{
    int row = ui.filters->currentRow();
    ui.filters->item(row, 0)->setText(FilterSettings::self()->filterFieldName(filter->filterField()));
    ui.filters->item(row, 0)->setData(32, filter->filterField());

    ui.filters->item(row, 1)->setText(FilterSettings::self()->filterTypeName(filter->filterType()));
    ui.filters->item(row, 1)->setData(32, filter->filterType());

    ui.filters->item(row, 2)->setText(filter->filterText());
    ui.filters->item(row, 2)->setData(32, filter->dontHideReplies());

    ui.filters->item(row, 3)->setText(FilterSettings::self()->filterActionName(filter->filterAction()));
    ui.filters->item(row, 3)->setData(32, filter->filterAction());
}

void ConfigureFilters::slotHideRepliesNotRelatedToMeToggled(bool enabled)
{
    if (enabled) {
        ui.cfg_hideNoneFriendsReplies->setChecked(false);
        ui.cfg_hideNoneFriendsReplies->setEnabled(false);
    } else {
        ui.cfg_hideNoneFriendsReplies->setEnabled(true);
        ui.cfg_hideNoneFriendsReplies->setChecked(FilterSettings::hideNoneFriendsReplies());
    }
}

#include "moc_configurefilters.cpp"
