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

#include "translatorconfig.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include <translatorsettings.h>
#include <QVBoxLayout>
#include <kstandarddirs.h>
#include <QFile>

K_PLUGIN_FACTORY( TranslatorConfigFactory, registerPlugin < TranslatorConfig > (); )
K_EXPORT_PLUGIN( TranslatorConfigFactory( "kcm_choqok_translator" ) )

TranslatorConfig::TranslatorConfig(QWidget* parent, const QVariantList& args):
        KCModule( TranslatorConfigFactory::componentData(), parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mTranslatorCtl");
    ui.setupUi(wd);
    addConfig( TranslatorSettings::self(), wd );
    layout->addWidget(wd);
    setButtons(KCModule::Apply | KCModule::Default);
    connect(ui.languagesList, SIGNAL(itemSelectionChanged()), SLOT(emitChanged()));

    missingLangs.insert("zh-CN", i18nc("Translation Language", "Chinese Simplified"));
    missingLangs.insert("zh-TW", i18nc("Translation Language", "Chinese Traditional"));
    missingLangs.insert("tl", i18nc("Translation Language", "Filipino"));
    missingLangs.insert("ht", i18nc("Translation Language", "Haitian Creole"));
    missingLangs.insert("iw", i18nc("Translation Language", "Hebrew"));
    missingLangs.insert("no", i18nc("Translation Language", "Norwegian"));
}

TranslatorConfig::~TranslatorConfig()
{

}

void TranslatorConfig::defaults()
{
    KCModule::defaults();
}

void TranslatorConfig::load()
{
    kDebug();
    langs << "af" << "sq" << "ar" << "be" << "bg" << "ca" << "zh-CN" << "zh-TW" << "hr" << "cs" << "da" << "nl" << "en" << "et" << "tl" << "fi" << "fr" << "gl" << "de" << "el" << "ht" << "iw" << "hi" << "hu" << "is" << "id" << "ga" << "it" << "ja" << "lv" << "lt" << "mk" << "ms" << "mt" << "no" << "fa" << "pl" << "pt" << "ro" << "ru" << "sr" << "sk" << "sl" << "es" << "sw" << "sv" << "th" << "tr" << "uk" << "vi" << "cy" << "yi";
    QStringList selected = TranslatorSettings::languages();
    foreach(const QString& ln, langs){
        QString flag = KStandardDirs::locate( "locale", QString( "l10n/%1/flag.png" ).arg( ln.toLower() ) );
        KIcon icon;
        if ( QFile::exists( flag ) )
            icon.addPixmap(QPixmap( flag ));
        QString langStr = KGlobal::locale()->languageCodeToName(ln.toLower());
        QListWidgetItem* item = new QListWidgetItem(icon, langStr.isEmpty() ? missingLangs.value(ln) : langStr);
        item->setData(32, ln);
        ui.languagesList->addItem(item);
        if(selected.contains(ln))
            item->setSelected(true);
    }
    KCModule::load();
}

void TranslatorConfig::save()
{
    kDebug();
    QStringList selected;
    int count = ui.languagesList->count();
    for(int i = 0; i < count; ++i){
        if(ui.languagesList->item(i)->isSelected())
            selected<< ui.languagesList->item(i)->data(32).toString();
    }
    TranslatorSettings::setLanguages(selected);
    TranslatorSettings::self()->writeConfig();
    KCModule::save();
}

void TranslatorConfig::emitChanged()
{
    emit changed(true);
}

#include "translatorconfig.moc"
