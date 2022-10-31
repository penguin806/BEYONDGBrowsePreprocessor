#include "mappingfromuniprot.h"
#include <QMap>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QRegularExpression>

MappingFromUniprot::MappingFromUniprot(QObject *parent) : QObject(parent)
{
    this->netManager = new QNetworkAccessManager(this);

    QObject::connect(this->netManager,SIGNAL(finished(QNetworkReply*)),
                     this,SLOT(onRequestUniprotFinished(QNetworkReply*)));
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

QString MappingFromUniprot::generateUniprotQueryString()
{
    QStringList proteinIdList;
    QMap<QString,QString>::const_iterator i = Map_EnsemblProtein_UniProtKB.constBegin();

    while(i != Map_EnsemblProtein_UniProtKB.constEnd())
    {
        proteinIdList.append(i.key());
        ++i;
    }

    QUrlQuery queryUniprotParam;
    queryUniprotParam.addQueryItem("from","ENSEMBL_PRO_ID");
    queryUniprotParam.addQueryItem("to","ID");
    queryUniprotParam.addQueryItem("format","tab");
    queryUniprotParam.addQueryItem("query",proteinIdList.join('+'));
    //queryUniprotParam.addQueryItem("query","ENSP00000334393");

#ifdef QT_DEBUG
    qDebug() << queryUniprotParam.query();
#endif

    return queryUniprotParam.query();
}

void MappingFromUniprot::parseResponseText(QString decodedResponse)
{
    QStringList responseLineList =
            decodedResponse.split(QRegularExpression("[\r\n]"),QString::SkipEmptyParts);

    QStringList::const_iterator const_iterator = responseLineList.constBegin();
    while (const_iterator != responseLineList.constEnd()) {
        QStringList bufferList = (*const_iterator).split('\t');

        if(bufferList.size() == 2)
        {
            if(this->Map_EnsemblProtein_UniProtKB.contains(bufferList.at(0)))
            {
                this->Map_EnsemblProtein_UniProtKB.insert(bufferList.at(0),
                                                          bufferList.at(1));
            }
        }

        ++const_iterator;
    }
}

void MappingFromUniprot::startRequestToQueryUniprot()
{
    QUrl uniprotUrl("https://legacy.uniprot.org/uploadlists/");
    QByteArray uniprotQuery = this->generateUniprotQueryString().toUtf8();

    QNetworkRequest request(uniprotUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Qt admin@xuefeng.space");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentLengthHeader, uniprotQuery.length());
    //request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant());
    //request.setRawHeader("Connection","close");
    //request.setRawHeader("Accept-Encoding","identity");

    QNetworkReply *reply = netManager->post(request, uniprotQuery);

    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                     this,SLOT(onRequestErrorOccurred(QNetworkReply::NetworkError)));

#ifdef QT_DEBUG
    qDebug() << "SSL Version:" << QSslSocket::sslLibraryBuildVersionString();
#endif
}

void MappingFromUniprot::onRequestUniprotFinished(QNetworkReply *reply)
{
#ifdef QT_DEBUG
    qDebug() << "Status: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "ErrorString: " << reply->errorString();
#endif

    if(reply->hasRawHeader("Location"))
    {
        QString pathToRedirect = QString::fromUtf8(reply->rawHeader("Location"));
        QUrl uniprotRedirectedUrl("https://legacy.uniprot.org/");
        uniprotRedirectedUrl.setPath(pathToRedirect);

        QNetworkRequest secondRequest(uniprotRedirectedUrl);
        secondRequest.setHeader(QNetworkRequest::UserAgentHeader, "Qt admin@xuefeng.space");
        //secondRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        //secondRequest.setHeader(QNetworkRequest::ContentLengthHeader, "0");

        this->netManager->get(secondRequest);
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QString decodedResponse = QString::fromUtf8(responseData);
    if(decodedResponse.size() <= 3)
    {
        emit uniprotMappingFinished(MappingFromUniprot::MAPPING_FAIL,
                                    "Nothing was responsed from the server");
        return;
    }
    this->parseResponseText(decodedResponse);

#ifdef QT_DEBUG
    qDebug() << "Response: " << responseData;
#endif

    reply->deleteLater();
    QTextStream bufFormatStream(&decodedResponse,QIODevice::ReadOnly);
    QString top10LineOfDecodedResponse;
    QString tempString;
    for(int lineCount = 0; bufFormatStream.readLineInto(&tempString) && lineCount < 10;
        lineCount++)
    {
        top10LineOfDecodedResponse.append(tempString + "\n");
    }

    emit uniprotMappingFinished(MappingFromUniprot::MAPPING_SUCCESS,
                                top10LineOfDecodedResponse);
}

void MappingFromUniprot::onRequestErrorOccurred(QNetworkReply::NetworkError errorCode)
{
#ifdef QT_DEBUG
    qDebug() << "ErrorCode: "<< errorCode;
#endif
}
