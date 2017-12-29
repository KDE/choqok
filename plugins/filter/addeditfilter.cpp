/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "addeditfilter.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

#include "filtersettings.h"

AddEditFilter::AddEditFilter(QWidget *parent, Filter *filter)
    : QDialog(parent), currentFilter(filter)
{
    ui.setupUi(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddEditFilter::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddEditFilter::reject);
    ui.formLayout->addWidget(buttonBox);

    connect(ui.filterAction, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &AddEditFilter::slotFilterActionChanged);

    setupFilterFields();
    setupFilterTypes();
    setupFilterActions();

    if (filter) {
        //qDebug() << filter->filterField();
        //Editing
        ui.filterField->setCurrentIndex(ui.filterField->findData(filter->filterField()));
        ui.filterType->setCurrentIndex(ui.filterType->findData(filter->filterType()));
        ui.filterAction->setCurrentIndex(ui.filterAction->findData(filter->filterAction()));
        ui.filterText->setText(filter->filterText());
        ui.dontHideReplies->setChecked(filter->dontHideReplies());
        setWindowTitle(i18n("Modify filter rules"));
    }
    ui.filterText->setFocus();

}

AddEditFilter::~AddEditFilter()
{
}

void AddEditFilter::accept()
{
    Filter::FilterField field =
        (Filter::FilterField) ui.filterField->itemData(ui.filterField->currentIndex()).toInt();
    Filter::FilterType type =
        (Filter::FilterType) ui.filterType->itemData(ui.filterType->currentIndex()).toInt();
    Filter::FilterAction action =
        (Filter::FilterAction) ui.filterAction->itemData(ui.filterAction->currentIndex()).toInt();
    QString fText = ui.filterText->text();
    bool dontHideReplies = ui.dontHideReplies->isChecked();
    if (currentFilter) {
        currentFilter->setFilterField(field);
        currentFilter->setFilterText(fText);
        currentFilter->setFilterType(type);
        currentFilter->setFilterAction(action);
        currentFilter->setDontHideReplies(dontHideReplies);
        Q_EMIT filterUpdated(currentFilter);
    } else {
        currentFilter = new Filter(fText, field, type, action, dontHideReplies);
        Q_EMIT newFilterRegistered(currentFilter);
    }
    QDialog::accept();
}

void AddEditFilter::slotFilterActionChanged(int index)
{
    ui.dontHideReplies->setVisible((Filter::FilterAction)ui.filterAction->itemData(index).toInt() == Filter::Remove);
}

void AddEditFilter::setupFilterFields()
{
    const QMap<Filter::FilterField, QString> fields = FilterSettings::filterFieldsMap();
    for (const Filter::FilterField &field: fields.keys()) {
        ui.filterField->addItem(fields.value(field), field);
    }
}

void AddEditFilter::setupFilterTypes()
{
    const QMap<Filter::FilterType, QString> types = FilterSettings::filterTypesMap();
    for (const Filter::FilterType &type: types.keys()) {
        ui.filterType->addItem(types.value(type), type);
    }
}

void AddEditFilter::setupFilterActions()
{
    const QMap<Filter::FilterAction, QString> actions = FilterSettings::filterActionsMap();
    for (const Filter::FilterAction &action: actions.keys()) {
        ui.filterAction->addItem(actions.value(action), action);
    }
}
