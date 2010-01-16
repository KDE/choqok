#ifndef UR1_CA_H
#define UR1_CA_H

#include <shortener.h>
#include <QString>
#include <QVariantList>
/**
@author Bhaskar Kandiyal \<bkandiyal@gmail.com\>
*/
class Ur1_ca : public Choqok::Shortener
{
    Q_OBJECT
public:
    Ur1_ca( QObject* parent, const QVariantList& args  );
    ~Ur1_ca();
public:
    QString shorten( const QString &url );
};

#endif
