#include "fileprocessingthread.h"
#include "mappingfromuniprot.h"
#include <QFile>
#include <QTextStream>
#include <QThread>

FileProcessingThread::FileProcessingThread(QString inputFilePath,
                                           QString outputFilePath,
                                           QObject *parent) : QThread(parent)
{
    this->inputFilePath = inputFilePath;
    this->outputFilePath = outputFilePath;
}

void FileProcessingThread::onUniprotMappingFinished(QString responseText)
{
    emit progressUpdated("Request Finished!\nResponse from Uniprot:\n" + responseText);


    emit progressUpdated("Thread exited. [Id: " +
                         QString::number(quintptr(QThread::currentThreadId())) + "]");
    this->mapUniprot->deleteLater();
    this->quit();
}

void FileProcessingThread::run()
{
    emit progressUpdated("Thread started. [Id: " +
                         QString::number(quintptr(QThread::currentThreadId())) + "]");

    this->mapUniprot = new MappingFromUniprot();
    QObject::connect(this->mapUniprot,SIGNAL(uniprotMappingFinished(QString)),
                     this,SLOT(onUniprotMappingFinished(QString)));

    this->processingInputFileAndWritingToTempFile();
    // Finished: Reading from input file and Writing to output file!
    // Next: proteinId -> query from uniprot -> uniprotKB
    this->sleep(1);
    this->mapUniprot->startRequestToQueryUniprot();
    emit progressUpdated("Remove duplicated protein_id finished!\nStart query from Uniprot...");

    this->exec();
}

void FileProcessingThread::processingInputFileAndWritingToTempFile()
{
    QFile inputFile(inputFilePath);
    if( !inputFile.open(QFile::ReadOnly) )
    {
        emit errorOccured("Unable to open file");
        return;
    }
    else
    {
        emit progressUpdated("Open input file <" + inputFilePath + "> Success!");
    }

    this->temporaryFilePath = outputFilePath + ".tmp";
    //QFile outputFile(outputFilePath);
    QFile tempOutputFile(this->temporaryFilePath);
    if( !tempOutputFile.open(QFile::WriteOnly) )
    {
        emit errorOccured("Distination dir is read-only");
        return;
    }
    else
    {
        emit progressUpdated("Create temporary file <" + temporaryFilePath + "> Success!");
    }

    QTextStream inputFileStream(&inputFile);
    QTextStream outputFileStream(&tempOutputFile);
    QString stringBuffer;
    qint32 loopCounts = 0;

    // Fetch each line from the file
    while(inputFileStream.readLineInto(&stringBuffer))
    {
        if(stringBuffer.at(0) == '#') // Comments, skip
        {
            continue;
        }
        //this->ui->textEdit_Info->append(stringBuffer);

        QStringList dataFields;
        dataFields = stringBuffer.split('\t');
        if(dataFields.length() < 9)
        {
            emit errorOccured("Not a valid gtf file");
            return;
        }

        // Parse each data field of the line
        GtfStructure gtfFields;
        gtfFields.seqName = dataFields.at(0);
        gtfFields.source = dataFields.at(1);
        gtfFields.feature = dataFields.at(2);
        gtfFields.start = dataFields.at(3);
        gtfFields.end = dataFields.at(4);
        gtfFields.score = dataFields.at(5);
        gtfFields.strand = dataFields.at(6);
        gtfFields.frame = dataFields.at(7);
        gtfFields.attributes = dataFields.at(8).split(';',QString::SkipEmptyParts);

        // Save fields useful to <outputFile> only

        QString attributeProteinId; // Need protein_id only
        attributeProteinId = gtfFields.attributes.
                filter("protein_id",Qt::CaseInsensitive).join(',');

        loopCounts++;
        if(attributeProteinId.isEmpty())
        {
            // protein_id Not Found, Skip
        }
        else
        {
            QString proteinIdExtracted =
                    this->extractProteinId(attributeProteinId).toString();

            mapUniprot->insertIntoProteinMap(proteinIdExtracted,QString());
            // Insert proteinId into map, preparing for query from uniprot

            stringBuffer = gtfFields.seqName + ',' + gtfFields.start
                    + ',' +gtfFields.end + ',' + proteinIdExtracted;
            outputFileStream << stringBuffer << '\n';

            // emit progressUpdated(stringBuffer);
            if( 0 == loopCounts % 500)
            {
                emit progressUpdated("Processing the " +
                                     QString::number(loopCounts) +
                                     "th line: " + stringBuffer);
            }
        }

    }

    // Finished removing useless columns, and wrote to TemporaryFile
    // Protein_id map init finished

}

QStringRef FileProcessingThread::extractProteinId(QString attrProteinId)
{
    // Example input: protein_id "ENSP00000493376.2"
    // Output: ENSP00000493376 (No version)
    int firstDoubleQuotationPos = attrProteinId.indexOf('\"');
    int lastDoubleQuotationPos = attrProteinId.lastIndexOf('\"');

    if(firstDoubleQuotationPos == -1
            ||lastDoubleQuotationPos == -1
            ||(firstDoubleQuotationPos+1) >= lastDoubleQuotationPos)
    {
        emit errorOccured("Attribute protein_id invalid");
        return QStringRef();
    }
    else
    {
        int lastPointPos = attrProteinId.lastIndexOf('.');
        if(lastPointPos < lastDoubleQuotationPos &&
                lastPointPos > firstDoubleQuotationPos)
        {
            return attrProteinId.midRef(firstDoubleQuotationPos + 1,
                    lastPointPos - firstDoubleQuotationPos - 1);
        }
        else
        {
            return attrProteinId.midRef(firstDoubleQuotationPos + 1,
                    lastDoubleQuotationPos - firstDoubleQuotationPos - 1);
        }
    }
}
