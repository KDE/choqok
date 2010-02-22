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

#include "twitpicuploadimage.h"
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <KIO/Job>
#include <kmimetype.h>
#include <notifymanager.h>
#include <KMessageBox>
#include <twitpicsettings.h>
#include <QDomDocument>
#include <choqokuiglobal.h>
#include <choqoktextedit.h>
#include <passwordmanager.h>

TwitpicUploadImage::TwitpicUploadImage(QWidget* parent)
    : KDialog(parent)
{
    setWindowTitle(i18n("Upload image to Twitpic"));
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);
    setAttribute(Qt::WA_DeleteOnClose);
    if( TwitpicSettings::username().isEmpty() ) {
        TwitpicSettings::self()->readConfig();
        if( TwitpicSettings::username().isEmpty() ){
            KMessageBox::sorry( Choqok::UI::Global::mainWindow(),
                                i18n("You did not set your twitter account.\nIn order to use this plugin, you have to set a twitter account: please go to Plugin Configuration and set it.") );
        }
    }

    kcfg_message = new Choqok::UI::TextEdit(0, widget);
    ui.gridLayout->addWidget(kcfg_message, 2, 1);
    connect(kcfg_message, SIGNAL(returnPressed(QString)), this, SLOT(submitImage()));
    resize(300,200);
}

TwitpicUploadImage::~TwitpicUploadImage()
{

}

void TwitpicUploadImage::slotButtonClicked(int button)
{
    if(button == KDialog::Ok){
        submitImage();
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void TwitpicUploadImage::submitImage()
{
    hide();
    QByteArray picData;
    QString tmp;
    KUrl picUrl(ui.kcfg_imageUrl->url());
    KIO::TransferJob *picJob = KIO::get( picUrl, KIO::Reload, KIO::HideProgressInfo);
    if( !KIO::NetAccess::synchronousRun(picJob, 0, &picData) ){
        kError()<<"Job error: " << picJob->errorString();
        KMessageBox::detailedError(this, i18n( "Uploading medium failed: cannot read the medium file." ),
                                               picJob->errorString() );
                                               return;
    }
    if ( picData.count() == 0 ) {
        kError() << "Cannot read the media file, please check if it exists.";
        KMessageBox::error( this, i18n( "Uploading medium failed: cannot read the medium file." ) );
        return;
    }
    ///Documentation: http://twitpic.com/api.do
    KUrl url( "http://twitpic.com/api/uploadAndPost" );
    QByteArray newLine("\r\n");
    QString formHeader( newLine + "Content-Disposition: form-data; name=\"%1\"" );
    QByteArray header(newLine + "--AaB03x");
    QByteArray footer(newLine + "--AaB03x--");
    QByteArray fileContentType = KMimeType::findByUrl( picUrl, 0, true )->name().toUtf8();
    QByteArray fileHeader(newLine + "Content-Disposition: file; name=\"media\"; filename=\"" +
    picUrl.fileName().toUtf8()+"\"");
    QByteArray data;
    data.append(header);

    data.append(fileHeader);
    data.append(newLine + "Content-Type: " + fileContentType);
    data.append(newLine);
    data.append(newLine + picData);

    data.append(header);
    data.append(formHeader.arg("username").toLatin1());
    data.append(newLine);
    data.append(newLine + TwitpicSettings::username().toLatin1());

    data.append(header);
    data.append(formHeader.arg("password").toLatin1());
    data.append(newLine);
    data.append(newLine + Choqok::PasswordManager::self()->readPassword(QString("twitpic_%1").arg(TwitpicSettings::username())).toUtf8());

    data.append(header);
    data.append(formHeader.arg("message").toLatin1());
    data.append(newLine);
    data.append(newLine + kcfg_message->toPlainText().toUtf8());

    data.append(footer);

    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo) ;
    if ( !job ) {
        kError() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData( "content-type", "Content-Type: multipart/form-data; boundary=AaB03x" );
    connect( job, SIGNAL( result( KJob* ) ),
             SLOT( slotTwitPicCreatePost(KJob*) ) );
    job->start();
}

void TwitpicUploadImage::slotTwitPicCreatePost( KJob *job )
{
    kDebug();
    if ( job->error() ) {
        kError() << "Job Error: " << job->errorString();
        KMessageBox::detailedError( Choqok::UI::Global::mainWindow(),
                                    i18n("Uploading image to Twitpic failed."),
                                    job->errorString() );
        show();
        return;
    } else {
        QDomDocument doc;
        QByteArray buffer = qobject_cast<KIO::StoredTransferJob*>(job)->data();
        doc.setContent(buffer);
        QDomElement element = doc.documentElement();
        if( element.tagName() == "rsp" ) {
            QString result;
            if(element.hasAttribute("stat") )
                result = element.attribute("stat" , "fail");
            else if(element.hasAttribute("status"))
                result = element.attribute("status" , "fail");
            else {
                kError()<<"Twitpic uploading failed: There isn't any \"stat\" or \"status\" attribute. Buffer:\n" << buffer;
                show();
                return;
            }
            if( result == "ok" ) {
                Choqok::NotifyManager::success( i18n("Image successfully uploaded to Twitpic, and posted to Twitter.") );
                close();
                return;
            } else {
                QDomNode node = element.firstChild();
                while( !node.isNull() ){
                    element = node.toElement();
                    if(element.tagName() == "err") {
                        QString err = element.attribute( "msg", i18n("Unrecognized result.") );
                        kDebug()<<"Server Error: "<<err;
                        KMessageBox::detailedError( Choqok::UI::Global::mainWindow(),
                                                    i18n("Uploading image to Twitpic failed."),
                                                    err );
                    }
                    node = node.nextSibling();
                }
                show();
                return;
            }
        } else {
            kError()<<"There isn't any \"rsp\" tag. Buffer:\n"<<buffer;
            KMessageBox::detailedError( Choqok::UI::Global::mainWindow(),
                                        i18n("Uploading image to Twitpic failed."),
                                        i18n("Unrecognized result.") );
            show();
        }
    }
}

#include "twitpicuploadimage.moc"
