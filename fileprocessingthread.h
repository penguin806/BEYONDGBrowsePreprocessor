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
    enum InputFileFormat {
        INPUT_FILE_GTF,
        INPUT_FILE_MSALIGN
    };

    FileProcessingThread(int inputFileFormat, QString inputFilePath, QString outputFilePath, QObject *parent = Q_NULLPTR);

signals:
    void progressUpdated(QString newLine);
    void errorOccured(QString errorReason);

public slots:
    //void onUniprotMappingFinished(MappingFromUniprot::MappingResult result, QString responseText);
    void onUniprotMappingFinished(int result, QString responseText);

protected:
    virtual void run();
    void processingInputFileAndWritingToTempFile();
    void processingTempFileAndWritingToFinalOutputFile();
    QStringRef extractProteinId(QString attrProteinId);

    void processingMsalignInputFileAndWritingToOutputFile();

private:
    int inputFileFormat;
    QString inputFilePath;
    QString outputFilePath;
    QString temporaryFilePath;
    MappingFromUniprot *mapUniprot;
};

#endif // FILEPROCESSINGTHREAD_H
