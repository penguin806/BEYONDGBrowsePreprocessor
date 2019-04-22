#include "mappingfromuniprot.h"
#include <QMap>

MappingFromUniprot::MappingFromUniprot(QObject *parent) : QObject(parent)
{
    this->netManager = new QNetworkAccessManager(this);
}

MappingFromUniprot::~MappingFromUniprot()
{
    this->netManager->deleteLater();
}

QString MappingFromUniprot::getValueFromProteinMap(const QString &key) const
{
    return this->Map_EnsemblProtein_UniProtKB.value(key);
}

void MappingFromUniprot::insertIntoProteinMap(const QString &key, const QString &value)
{
    this->Map_EnsemblProtein_UniProtKB.insert(key, value);
}
