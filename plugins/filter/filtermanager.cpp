/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "filtermanager.h"
#include <KAction>
#include <KActionCollection>
#include <KAboutData>
#include <KGenericFactory>
#include <choqokuiglobal.h>
#include <quickpost.h>
#include <KMessageBox>
#include <QTimer>
#include "filtersettings.h"
#include "filter.h"
#include "configurefilters.h"
#include <postwidget.h>
#include <twitterapihelper/twitterapiaccount.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < FilterManager > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_filter" ) )

FilterManager::FilterManager(QObject* parent, const QList<QVariant>& )
        :Choqok::Plugin(MyPluginFactory::componentData(), parent), state(Stopped)
{
    kDebug();
    KAction *action = new KAction(i18n("Configure Filters..."), this);
    actionCollection()->addAction("configureFilters", action);
    connect(action, SIGNAL(triggered(bool)), SLOT(slotConfigureFilters()));
    setXMLFile("filterui.rc");
    connect( Choqok::UI::Global::SessionManager::self(),
            SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
            SLOT(slotAddNewPostWidget(Choqok::UI::PostWidget*)) );
}

FilterManager::~FilterManager()
{

}


void FilterManager::slotAddNewPostWidget(Choqok::UI::PostWidget* newWidget)
{
//     if(!theAccount->inherits("TwitterApiAccount")){
//         kDebug()<<"Not a TwitterApi like account";
//         return;
//     }
    postsQueue.enqueue(newWidget);
    if(state == Stopped){
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

void FilterManager::startParsing()
{
    int i = 8;
    while( !postsQueue.isEmpty() && i>0 ){
        parse(postsQueue.dequeue());
        --i;
    }

    if(postsQueue.isEmpty())
        state = Stopped;
    else
        QTimer::singleShot(500, this, SLOT(startParsing()));
}

void FilterManager::parse(Choqok::UI::PostWidget* postToParse)
{
    if(!postToParse)
        return;

    if( parseSpecialRules(postToParse) )
        return;

    if(!postToParse)
        return;

    foreach(Filter* filter, FilterSettings::self()->filters()) {
        if(filter->filterText().isEmpty())
            return;
        switch(filter->filterField()){
            case Filter::Content:
                doFiltering( postToParse, filterText(postToParse->currentPost().content, filter) );
                break;
            case Filter::AuthorUsername:
                doFiltering( postToParse, filterText(postToParse->currentPost().author.userName, filter) );
                break;
            case Filter::ReplyToUsername:
                doFiltering( postToParse, filterText(postToParse->currentPost().replyToUserName, filter) );
                break;
            case Filter::Source:
                doFiltering( postToParse, filterText(postToParse->currentPost().source, filter) );
                break;
            default:
                break;
        };
    }
}

FilterManager::FilterAction FilterManager::filterText(const QString& textToCheck, Filter* filter)
{
    switch(filter->filterType()){
        case Filter::ExactMatch:
            if(textToCheck == filter->filterText()){
                kDebug()<<"ExactMatch";
                return Remove;
            }
            break;
        case Filter::RegExp:
            if( textToCheck.contains(QRegExp(filter->filterText())) ){
                kDebug()<<"RegExp";
                return Remove;
            }
            break;
        case Filter::Contain:
            if( textToCheck.contains(filter->filterText()) ){
                kDebug()<<"Contain";
                return Remove;
            }
            break;
        case Filter::DoesNotContain:
            if( !textToCheck.contains(filter->filterText()) ){
                kDebug()<<"DoesNotContain";
                return Remove;
            }
            break;
        default:
            return None;
            break;
    }
    return None;
}

void FilterManager::doFiltering(Choqok::UI::PostWidget* postToFilter, FilterManager::FilterAction action)
{
    switch(action){
        case Remove:
            kDebug()<<"Post filtered: "<<postToFilter->currentPost().content;
            postToFilter->close();
            break;
        default:
            //Do nothing
            break;
    }
}

void FilterManager::slotConfigureFilters()
{
    QPointer<ConfigureFilters> dlg = new ConfigureFilters(Choqok::UI::Global::mainWindow());
    dlg->show();
}

bool FilterManager::parseSpecialRules(Choqok::UI::PostWidget* postToParse)
{
    if(FilterSettings::hideRepliesNotRelatedToMe()){
        if( !postToParse->currentPost().replyToUserName.isEmpty() &&
            postToParse->currentPost().replyToUserName != postToParse->currentAccount()->username() ) {
            if( !postToParse->currentPost().content.contains(postToParse->currentAccount()->username()) ) {
                postToParse->close();
                kDebug()<<"NOT RELATE TO ME FILTERING......";
                return true;
            }
        }
    }

    if( FilterSettings::hideNoneFriendsReplies() ) {
        TwitterApiAccount *acc = qobject_cast<TwitterApiAccount*>(postToParse->currentAccount());
        if(!acc)
            return false;
        if( !postToParse->currentPost().replyToUserName.isEmpty() &&
            !acc->friendsList().contains(postToParse->currentPost().replyToUserName) ) {
            if( !postToParse->currentPost().content.contains(postToParse->currentAccount()->username()) ) {
                postToParse->close();
                kDebug()<<"NONE FRIEND FILTERING......";
                return true;
            }
        }
    }

    return false;
}


#include "filtermanager.moc"
