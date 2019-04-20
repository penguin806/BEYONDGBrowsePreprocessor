#ifndef FILEPROCESSINGTHREAD_H
#define FILEPROCESSINGTHREAD_H

#include <QThread>

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
    FileProcessingThread(QString inputFilePath, QString outputFilePath);

signals:
    void progressUpdated(QString newLine);
    void errorOccured(QString errorReason);

protected:
    virtual void run();
    QStringRef extractProteinId(QString attrProteinId);

private:
    QString inputFilePath;
    QString outputFilePath;
};

#endif // FILEPROCESSINGTHREAD_H
