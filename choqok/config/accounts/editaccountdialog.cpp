/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "editaccountdialog.h"

#include <QPushButton>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include "account.h"
#include "accountmanager.h"
#include "accountsdebug.h"
#include "editaccountwidget.h"

EditAccountDialog::EditAccountDialog(ChoqokEditAccountWidget *editWidget, QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags), widget(editWidget)
{
    if (!widget) {
        this->deleteLater();
        return;
    }

    setWindowTitle(i18n("Edit Account"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(widget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &EditAccountDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &EditAccountDialog::reject);
    mainLayout->addWidget(buttonBox);
}

EditAccountDialog::~EditAccountDialog()
{
}

void EditAccountDialog::accept()
{
    qCDebug(CHOQOK);
    if (widget->validateData()) {
        if (widget->apply()) {
            QDialog::accept();
        }
    } else {
      KMessageBox::error(
          this, i18n("Cannot validate your input information.\nPlease check "
                     "the fields' data.\nMaybe a required field is empty?"));
    }
}

