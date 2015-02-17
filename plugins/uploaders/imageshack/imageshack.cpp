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

#include <QDomDocument>

#include <KAboutData>
#include <KAction>
#include <KActionCollection>
#include <KGenericFactory>
#include <KIO/Job>
#include <KIO/NetAccess>
#include "choqokdebug.h"

#include "mediamanager.h"
#include "passwordmanager.h"
#include "shortenmanager.h"

const char * apiKey = "ZMWLXQBOfb570310607355f90c601148a3203f0f";

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < ImageShack > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_imageshack" ) )

ImageShack::ImageShack(QObject* parent, const QList<QVariant>& )
        :Choqok::Uploader(MyPluginFactory::componentData(), parent)
{
}

ImageShack::~ImageShack()
{
}

void ImageShack::upload(const QUrl &localUrl, const QByteArray& medium, const QByteArray& mediumType)
{
    qCDebug(CHOQOK);
    if( !mediumType.startsWith(QByteArray("image/")) ){
        Q_EMIT uploadingFailed(localUrl, i18n("Just supporting image uploading"));
        return;
    }
    QUrl url("http://www.imageshack.us/upload_api.php");
    QMap<QString, QByteArray> formdata;
    formdata["key"] = apiKey;
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
    qCDebug(CHOQOK);
    QUrl localUrl = mUrlMap.take( job );
    if ( job->error() ) {
        qCritical() << "Job Error: " << job->errorString();
        Q_EMIT uploadingFailed(localUrl, job->errorString());
        return;
    } else {
        KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
        QDomDocument doc;
        doc.setContent(stj->data());
        if(doc.firstChild().isNull()) {
            qCDebug(CHOQOK)<<"Malformed response: "<< stj->data();
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
        qCDebug(CHOQOK)<<"Response not detected: "<<stj->data();
    }
}

/*
 * Return XML:
<?xml version="1.0" encoding="iso-8859-1"?><imginfo xmlns="http://ns.imageshack.us/imginfo/7/" version="7" timestamp="1286536790">
  <rating>
    <ratings>0</ratings>
    <avg>0.0</avg>
  </rating>
  <files server="717" bucket="1094">
     <image size="6913" content-type="image/jpeg">mehrdad5050.jpg</image>
  </files>
  <resolution>
    <width>50</width>
    <height>50</height>
  </resolution>
  <exif-info>
 <exifmake>DIGITAL</exifmake>
 <exifmodel>F15</exifmodel>
 <exiforientation>Horizontal (normal)</exiforientation>
 <exifflash>Fired</exifflash>
 <exiffocallength>5.6 mm</exiffocallength>
 <exifdatetime>2036-02-09 11:36:19</exifdatetime>
 <exifexposuretime>1/10</exifexposuretime>
 <exiflensfnum>f/3.5</exiflensfnum>
 <exifaperture>f/3.5</exifaperture>
 <exifiso>100</exifiso>
 <exifmeteringmode>Center-weighted average</exifmeteringmode>
 <exifexposureprogram>Program AE</exifexposureprogram>
 <exifcompression>JPEG (old-style)</exifcompression>
  </exif-info>
  <class>r</class>
  <visibility>no</visibility>
  <uploader>
    <ip>85.185.37.196</ip>
  </uploader>
  <links>
    <image_link>http://img717.imageshack.us/img717/1094/mehrdad5050.jpg</image_link>
    <image_html>&lt;a href=&quot;http://img717.imageshack.us/my.php?image=mehrdad5050.jpg&quot; target=&quot;_blank&quot;&gt;&lt;img src=&quot;http://img717.imageshack.us/img717/1094/mehrdad5050.jpg&quot; alt=&quot;Free Image Hosting at www.ImageShack.us&quot; border=&quot;0&quot;/&gt;&lt;/a&gt;</image_html>
    <image_bb>[URL=http://img717.imageshack.us/my.php?image=mehrdad5050.jpg][IMG]http://img717.imageshack.us/img717/1094/mehrdad5050.jpg[/IMG][/URL]</image_bb>
    <image_bb2>[url=http://img717.imageshack.us/my.php?image=mehrdad5050.jpg][img=http://img717.imageshack.us/img717/1094/mehrdad5050.jpg][/url]</image_bb2>
    <thumb_html>&lt;a href=&quot;http://img717.imageshack.us/my.php?image=mehrdad5050.jpg&quot; target=&quot;_blank&quot;&gt;&lt;img src=&quot;http://www.imageshack.us/thumbnail.png&quot; alt=&quot;Free Image Hosting at www.ImageShack.us&quot; border=&quot;0&quot;/&gt;&lt;/a&gt;</thumb_html>
    <thumb_bb>[URL=http://img717.imageshack.us/my.php?image=mehrdad5050.jpg][IMG]http://www.imageshack.us/thumbnail.png[/IMG][/URL]</thumb_bb>
    <thumb_bb2>[url=http://img717.imageshack.us/my.php?image=mehrdad5050.jpg][img=http://www.imageshack.us/thumbnail.png][/url]</thumb_bb2>
    <yfrog_link>http://yfrog.com/jxmehrdad5050j</yfrog_link>
    <yfrog_thumb>http://yfrog.com/jxmehrdad5050j.th.jpg</yfrog_thumb>
    <ad_link>http://img717.imageshack.us/my.php?image=mehrdad5050.jpg</ad_link>
    <done_page>http://img717.imageshack.us/content.php?page=done&amp;l=img717/1094/mehrdad5050.jpg</done_page>
  </links>
</imginfo>

ERROR response format:
<links>
<error id="wrong_file_type">Wrong file type detected for file tw:application/x-shellscript</error>
</links>

*/

