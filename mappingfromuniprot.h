#ifndef MAPPINGFROMUNIPROT_H
#define MAPPINGFROMUNIPROT_H

#include <QObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class MappingFromUniprot : public QObject
{
    Q_OBJECT
public:
    explicit MappingFromUniprot(QObject *parent = nullptr);
    ~MappingFromUniprot();

    QString getValueFromProteinMap(const QString &key) const;
    void insertIntoProteinMap(const QString &key, const QString &value);
    void startRequestToQueryUniprot();

signals:
    void uniprotMappingFinished(QString responseText);

public slots:
    void onRequestUniprotFinished(QNetworkReply *reply);
    void onRequestErrorOccurred(QNetworkReply::NetworkError errorCode);

private:
    QMap<QString,QString> Map_EnsemblProtein_UniProtKB;
    QNetworkAccessManager *netManager;
    QString generateUniprotQueryString();
    void parseResponseText(QString decodedResponse);
};

#endif // MAPPINGFROMUNIPROT_H
