/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "addaccountdialog.h"
#include "editaccountwidget.h"
#include <klocalizedstring.h>
#include <accountmanager.h>
#include <KMessageBox>
#include <KDebug>

AddAccountDialog::AddAccountDialog( ChoqokEditAccountWidget *addWidget,QWidget* parent, Qt::WFlags flags)
        : KDialog(parent, flags), widget(addWidget)
{
    if(!widget) {
        this->deleteLater();
        return;
    }
    setMainWidget(widget);
    setCaption(i18n("Add New Account"));
}

AddAccountDialog::~AddAccountDialog()
{

}

void AddAccountDialog::closeEvent(QCloseEvent* e)
{
    KDialog::closeEvent(e);
}

void AddAccountDialog::slotButtonClicked(int button)
{
    kDebug()<<button;
    if(button == KDialog::Ok) {
        if( widget->validateData() ){
            if( Choqok::Account *acc = widget->apply() ) {
                if( !Choqok::AccountManager::self()->registerAccount( acc ) )
                    KMessageBox::detailedError( this, i18n("The Account registration failed."),
                                                Choqok::AccountManager::self()->lastError() );
                else
                    accept();
            }
        } else {
            KMessageBox::sorry(this, i18n("Cannot validate your input information.\nPlease check the fields' data.\nMaybe a required field is empty?"));
        }
    } else {
        Choqok::AccountManager::self()->removeAccount(widget->account()->alias());
        KDialog::slotButtonClicked(button);
    }
}
