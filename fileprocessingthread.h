#ifndef FILEPROCESSINGTHREAD_H
#define FILEPROCESSINGTHREAD_H

#include <QThread>
#include "mappingfromuniprot.h"

struct gtfStruct
{
    gtfStruct() {}
    QString seqName;
    QString source;
    QString feature;
    QString start;
    QString end;
    QString score;
    QString strand;
    QString frame;
    QStringList attributes;
};
typedef gtfStruct GtfStructure;

class FileProcessingThread : public QThread
{
    Q_OBJECT
public:
    FileProcessingThread(QString inputFilePath, QString outputFilePath, QObject *parent = Q_NULLPTR);

signals:
    void progressUpdated(QString newLine);
    void errorOccured(QString errorReason);

public slots:
    void onUniprotMappingFinished(QString responseText);

protected:
    virtual void run();
    void processingInputFileAndWritingToTempFile();
    QStringRef extractProteinId(QString attrProteinId);

private:
    QString inputFilePath;
    QString outputFilePath;
    QString temporaryFilePath;
    MappingFromUniprot *mapUniprot;
};

#endif // FILEPROCESSINGTHREAD_H
