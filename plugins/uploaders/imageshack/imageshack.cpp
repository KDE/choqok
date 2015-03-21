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

#include "imageshack.h"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include <KIO/Job>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KPluginFactory>

#include "mediamanager.h"

const static QString apiKey("ZMWLXQBOfb570310607355f90c601148a3203f0f");

K_PLUGIN_FACTORY_WITH_JSON( ImageShackFactory, "choqok_imageshack.json",
                            registerPlugin < ImageShack > (); )

ImageShack::ImageShack(QObject* parent, const QList<QVariant>& )
    :Choqok::Uploader("choqok_imageshack", parent)
{
}

ImageShack::~ImageShack()
{
}

void ImageShack::upload(const QUrl &localUrl, const QByteArray& medium, const QByteArray& mediumType)
{
    if( !mediumType.startsWith(QByteArray("image/")) ){
        Q_EMIT uploadingFailed(localUrl, i18n("Just supporting image uploading"));
        return;
    }
    QUrl url("http://www.imageshack.us/upload_api.php");
    QMap<QString, QByteArray> formdata;
    formdata["key"] = apiKey.toLatin1();
    formdata["rembar"] = "1";

    QMap<QString, QByteArray> mediafile;
    mediafile["name"] = "fileupload";
    mediafile["filename"] = localUrl.fileName().toUtf8();
    mediafile["mediumType"] = mediumType;
    mediafile["medium"] = medium;
    QList< QMap<QString, QByteArray> > listMediafiles;
    listMediafiles.append(mediafile);

    QByteArray data = Choqok::MediaManager::createMultipartFormData( formdata, listMediafiles );

    KIO::StoredTransferJob *job = KIO::storedHttpPost( data, url, KIO::HideProgressInfo ) ;
    if ( !job ) {
        qCritical() << "Cannot create a http POST request!";
        return;
    }
    job->addMetaData( "content-type", "Content-Type: multipart/form-data; boundary=AaB03x" );
    mUrlMap[job] = localUrl;
    connect( job, SIGNAL( result( KJob* ) ),
             SLOT( slotUpload(KJob*)) );
    job->start();
}

void ImageShack::slotUpload(KJob* job)
{
    QUrl localUrl = mUrlMap.take( job );
    if ( job->error() ) {
        qCritical() << "Job Error: " << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
        QDomDocument doc;
        doc.setContent(stj->data());
        if (doc.firstChild().isNull()) {
            qWarning() << "Malformed response:" << stj->data();
            return;
        }
        QDomElement root = doc.documentElement();
        if(root.tagName() == "imginfo"){
            QDomNode node = root.firstChild();
            while(!node.isNull()){
                QDomElement elm = node.toElement();
                if(elm.tagName() == "links"){
                    QDomNode node2 = node.firstChild();
                    while( !node2.isNull() ){
                        QDomElement elm2 = node2.toElement();
                        if(elm2.tagName() == "yfrog_link"){
                            Q_EMIT mediumUploaded(localUrl, elm2.text());
                            return;
                        }
                        node2 = node2.nextSibling();
                    }
                }
                node = node.nextSibling();
            }
        } else {
            if (root.tagName() == "links"){
                QDomNode node = root.firstChild();
                if( !node.isNull() ){
                    if(node.toElement().tagName() == "error" ){
                        Q_EMIT uploadingFailed(localUrl, node.toElement().text());
                        return;
                    }
                }
            }
        }
        Q_EMIT uploadingFailed(localUrl, i18n("Malformed response"));
        qWarning() << "Response not detected:" << stj->data();
    }
}

#include "imageshack.moc"