/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef DATACONTAINERS_H
#define DATACONTAINERS_H
#include <QDateTime>

struct User{
public:
	uint userId;
	QString name;
	QString screenName;
	QString location;
	QString description;
	QString profileImageUrl;
	QString homePageUrl;
	bool isProtected;
	uint followersCount;
};

struct Status{
public:
	QDateTime creationDateTime;
	uint statusId;
	QString content;
	QString source;
	bool isTruncated;
	uint replyToStatusId;
	uint replyToUserId;
	bool isFavorited;
	QString replyToUserScreenName;
	User user;
	bool isError;
};

struct Account{
public:
  QString username;
  QString password;
  QString serviceName;
  QString alias;
  Qt::LayoutDirection direction;
  QString apiPath;
  bool isError;
};

#endif
