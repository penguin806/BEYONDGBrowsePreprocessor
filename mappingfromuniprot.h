#ifndef MAPPINGFROMUNIPROT_H
#define MAPPINGFROMUNIPROT_H

#include <QObject>
#include <QMap>
#include <QNetworkAccessManager>

class MappingFromUniprot : public QObject
{
    Q_OBJECT
public:
    explicit MappingFromUniprot(QObject *parent = nullptr);
    ~MappingFromUniprot();

    QString getValueFromProteinMap(const QString &key) const;
    void insertIntoProteinMap(const QString &key, const QString &value);

signals:

public slots:

private:
    QMap<QString,QString> Map_EnsemblProtein_UniProtKB;
    QNetworkAccessManager *netManager;
};

#endif // MAPPINGFROMUNIPROT_H
