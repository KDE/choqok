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

#include "filterconfig.h"
#include "filtersettings.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include <QVBoxLayout>
#include <KDebug>
#include "filter.h"

K_PLUGIN_FACTORY( FilteringConfigFactory, registerPlugin < FilteringConfig > (); )
K_EXPORT_PLUGIN( FilteringConfigFactory( "kcm_choqok_filter" ) )

FilteringConfig::FilteringConfig(QWidget* parent, const QVariantList& args):
        KCModule( FilteringConfigFactory::componentData(), parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mFilteringCtl");
    ui.setupUi(wd);
    layout->addWidget(wd);
//     setButtons(KCModule::Apply | KCModule::Default);
}

FilteringConfig::~FilteringConfig()
{

}

void FilteringConfig::defaults()
{
    KCModule::defaults();
}

void FilteringConfig::load()
{
    kDebug();
    KCModule::load();
}

void FilteringConfig::save()
{
    kDebug();
    KCModule::save();
}

void FilteringConfig::emitChanged()
{
    emit changed(true);
}

void FilteringConfig::reloadFiltersTable()
{
    ui.filters->clearContents();
    QList<Filter*> filters = FilterSettings::self()->availableFilters();
    foreach(Filter *filter, filters){
        int row = ui.filters->rowCount();
        ui.filters->insertRow(row);
        ui.filters->setItem(row, 0,
                            new QTableWidgetItem(FilterSettings::self()->filterFieldName()[filter->filterField()]));
        ui.filters->setItem(row, 1,
                            new QTableWidgetItem(FilterSettings::self()->filterTypeName()[filter->filterType()]));
        ui.filters->setItem(row, 2,
                            new QTableWidgetItem(filter->filterText()));
    }
}

void FilteringConfig::saveFiltersTable()
{
    QList<Filter*> list;
    int count = ui.filters->rowCount();
    for(int i=0; i<count; ++i){
        Filter::FilterField field = FilterSettings::self()->filterFieldName().key(ui.filters->item(i, 0)->text());
        Filter::FilterType type = FilterSettings::self()->filterTypeName().key(ui.filters->item(i, 1)->text());
        QString text = ui.filters->item(i, 2)->text();
        Filter *f = new Filter(text, field, type, FilterSettings::self());
        list << f;
    }
    FilterSettings::self()->setFilters(list);
}

#include "filterconfig.moc"
