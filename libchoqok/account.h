/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKACCOUNT_H
#define CHOQOKACCOUNT_H

#include <QString>

#include <KConfigGroup>
#include <memory>
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
    Account(MicroBlog *parent, const QString &alias);

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
    void setUsername(const QString &name);

    QString password() const;
    void setPassword(const QString &pass);

    QString alias() const;
    void setAlias(const QString &alias);

    bool isReadOnly() const;
    void setReadOnly(bool readonly = true);

    bool isEnabled() const;
    void setEnabled(bool enabled = true);

    bool showInQuickPost() const;
    void setShowInQuickPost(bool show = true);

    void setPostCharLimit(const uint limit);
    /**
    Indicate character limit for a post. 0 means no limit.
    */
    uint postCharLimit() const;

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
    void setPriority(uint priority);

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
    void modified(Choqok::Account *theAccount);
    void status(Choqok::Account *theAccount, bool enabled);

private:
    class Private;
    std::unique_ptr<Private> d;
};

}
#endif
