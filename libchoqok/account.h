/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#ifndef CHOQOKACCOUNT_H
#define CHOQOKACCOUNT_H

#include <QtCore/QString>
#include <kconfiggroup.h>
#include "choqok_export.h"

namespace Choqok
{
class MicroBlog;
/**
\brief Account class base
MicroBlog plugins can subclass this class or use it if fill their needs.

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT Account : public QObject
{
    Q_OBJECT
public:
    Account(MicroBlog *parent, const QString& alias);

    ~Account();
    /**
    \brief MicroBlog for this account
    */
    MicroBlog *microblog() const;

    /**
    By default this will return @ref microblog() 's timelineNames()
    Some microblogs may need to change this per account base!
    */
    virtual QStringList timelineNames() const;

    QString username() const;
    void setUsername( const QString &name );

    QString password() const;
    void setPassword( const QString &pass );

    QString alias() const;
    void setAlias( const QString &alias );

    bool isReadOnly() const;
    void setReadOnly(bool readonly = true);

    bool isEnabled() const;
    void setEnabled(bool enabled = true);

    bool showInQuickPost() const;
    void setShowInQuickPost(bool show = true);

    virtual void writeConfig();
    /**
    * \brief Get the priority of this account.
    *
    * Used for sorting mainwindow tab widgets.
    */
    uint priority() const;

    /**
    * \brief Set the priority of this account.
    *
    * @note This method is called by the UI, and should not be called elsewhere.
    */
    void setPriority( uint priority );

    /**
    * Return the @ref KConfigGroup used to write and read special properties
    *
    * "MicroBlog", "UserId", "Username" , "Password", "Priority", "Enabled" are reserved keyword
    * already in use in that group.
    *
    * for compatibility, try to not use key that start with a uppercase
    */
    KConfigGroup *configGroup() const;

Q_SIGNALS:
    void modified( Choqok::Account *theAccount );

private:
    class Private;
    Private * const d;
};

}
#endif
