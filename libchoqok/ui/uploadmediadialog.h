/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UPLOADMEDIADIALOG_H
#define UPLOADMEDIADIALOG_H

#include <QDialog>
#include <QUrl>

#include "choqok_export.h"

namespace Choqok
{

namespace UI
{

class CHOQOK_EXPORT UploadMediaDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UploadMediaDialog(QWidget *parent = nullptr, const QString &url = QString());
    ~UploadMediaDialog();

protected:
    void load();
    bool showed;

protected Q_SLOTS:
    virtual void accept() override;
    void currentPluginChanged(int index);
    void slotAboutClicked();
    void slotConfigureClicked();
    void slotMediumUploadFailed(const QUrl &localUrl, const QString &errorMessage);
    void slotMediumUploaded(const QUrl &localUrl, const QString &remoteUrl);
    void slotMediumChanged(const QString &url);

private:
    class Private;
    Private *const d;
    QSize winSize;
};

}

}

#endif // UPLOADMEDIADIALOG_H
