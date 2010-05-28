/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "configurefilters.h"
#include "filtersettings.h"
#include <klocale.h>
#include <qlayout.h>
#include <QVBoxLayout>
#include <KDebug>
#include "filter.h"
#include "addeditfilter.h"

ConfigureFilters::ConfigureFilters(QWidget* parent):
        KDialog(parent)
{
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mFilteringCtl");
    ui.setupUi(wd);
    setMainWidget(wd);
    resize(400, 300);

    setWindowTitle(i18n("Configure Filters"));

    ui.btnAdd->setIcon(KIcon("list-add"));
    ui.btnEdit->setIcon(KIcon("document-edit"));
    ui.btnRemove->setIcon(KIcon("list-remove"));
    connect( ui.btnAdd, SIGNAL(clicked()), SLOT(slotAddFilter()) );
    connect( ui.btnEdit, SIGNAL(clicked()), SLOT(slotEditFilter()));
    connect( ui.btnRemove, SIGNAL(clicked()), SLOT(slotRemoveFilter()));
    reloadFiltersTable();
}

ConfigureFilters::~ConfigureFilters()
{}

void ConfigureFilters::slotButtonClicked(int button)
{
    if(button == Ok){
        saveFiltersTable();
        accept();
    } else
        KDialog::slotButtonClicked(button);
}

void ConfigureFilters::reloadFiltersTable()
{
    ui.filters->clearContents();
    QList<Filter*> filters = FilterSettings::self()->filters();
    kDebug()<<filters.count();
    foreach(Filter *filter, filters){
        addNewFilter(filter);
    }
}

void ConfigureFilters::saveFiltersTable()
{
    QList<Filter*> list;
    int count = ui.filters->rowCount();
    for(int i=0; i<count; ++i){
        Filter::FilterField field = FilterSettings::self()->filterFieldFromName(ui.filters->item(i, 0)->text());
        Filter::FilterType type = FilterSettings::self()->filterTypeFromName(ui.filters->item(i, 1)->text());
        QString text = ui.filters->item(i, 2)->text();
        Filter *f = new Filter(text, field, type, FilterSettings::self());
        list << f;
    }
    FilterSettings::self()->setFilters(list);
    FilterSettings::self()->writeConfig();
}

void ConfigureFilters::slotAddFilter()
{
    AddEditFilter *f = new AddEditFilter(this);
    connect(f, SIGNAL(newFilterRegistered(Filter*)), SLOT(addNewFilter(Filter*)));
    f->show();
}

void ConfigureFilters::slotEditFilter()
{
    if(ui.filters->selectedItems().count()>0){
        int row = ui.filters->currentRow();
        Filter::FilterField field;
        Filter::FilterType type;
        field = (Filter::FilterField) ui.filters->item(row, 0)->data(32).toInt();
        type = (Filter::FilterType) ui.filters->item(row, 1)->data(32).toInt();
        QString text = ui.filters->item(row, 2)->text();
        Filter *f = new Filter(text, field, type, this);
        QPointer<AddEditFilter> dialog = new AddEditFilter(this, f);
        connect(dialog, SIGNAL(filterUpdated(Filter*)), SLOT(slotUpdateFilter(Filter*)));
        dialog->exec();
    }
}

void ConfigureFilters::slotRemoveFilter()
{
    if(ui.filters->selectedItems().count()>0){
        int row = ui.filters->currentRow();
//         int field = ui.filters->item(row, 0)->data(32).toInt();
//         int type = ui.filters->item(row, 1)->data(32).toInt();
//         QString text = ui.filters->item(row, 2)->text();
//         KGlobal::config()->deleteGroup(QString("%1%2%3").arg(text).arg(field).arg(type));
        ui.filters->removeRow(row);
    }
}

void ConfigureFilters::addNewFilter(Filter* filter)
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
    ui.filters->setItem(row, 2, item3);
}

void ConfigureFilters::slotUpdateFilter(Filter* filter)
{
    int row = ui.filters->currentRow();
    ui.filters->item(row, 0)->setText(FilterSettings::self()->filterFieldName(filter->filterField()));
    ui.filters->item(row, 0)->setData(32, filter->filterField());

    ui.filters->item(row, 1)->setText(FilterSettings::self()->filterTypeName(filter->filterType()));
    ui.filters->item(row, 1)->setData(32, filter->filterType());

    ui.filters->item(row, 2)->setText(filter->filterText());
}


#include "configurefilters.moc"
