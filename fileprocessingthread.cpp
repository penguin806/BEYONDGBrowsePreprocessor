#include "fileprocessingthread.h"
#include "mappingfromuniprot.h"
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QRegularExpression>

FileProcessingThread::FileProcessingThread(int inputFileFormat,
                                           QString inputFilePath,
                                           QString outputFilePath,
                                           QObject *parent) : QThread(parent)
{
    this->inputFileFormat = inputFileFormat;
    this->inputFilePath = inputFilePath;
    this->outputFilePath = outputFilePath;
}

void FileProcessingThread::onUniprotMappingFinished(int result, QString responseText)
{
    if(result == MappingFromUniprot::MAPPING_SUCCESS)
    {
        emit progressUpdated("Request Finished!\nResponse from Uniprot:\n" +
                             responseText + "\n...");
    }
    else
    {
        emit progressUpdated("Request Failed!\nThe internet connection may not available!");

        emit progressUpdated("Thread exited. [Id: " +
                             QString::number(quintptr(QThread::currentThreadId())) + "]");
        this->mapUniprot->deleteLater();
        this->quit();
        return;
    }


    this->sleep(1);
    this->processingTempFileAndWritingToFinalOutputFile();
    emit progressUpdated("Append mapping result to each line success!");

    emit progressUpdated("Thread exited. [Id: " +
                         QString::number(quintptr(QThread::currentThreadId())) + "]");
    this->mapUniprot->deleteLater();
    this->quit();
    return;
}

void FileProcessingThread::run()
{
    emit progressUpdated("Thread started. [Id: " +
                         QString::number(quintptr(QThread::currentThreadId())) + "]");
#ifdef QT_DEBUG
    qDebug() << QString("InputFileFormat: ") + QString::number(this->inputFileFormat);
    qDebug() << "InputFilePath: " + this->inputFilePath;
    qDebug() << "OutputFilePath: " + this->outputFilePath;
#endif

    if(this->inputFileFormat == FileProcessingThread::INPUT_FILE_GTF)
    {
        this->mapUniprot = new MappingFromUniprot();
        QObject::connect(this->mapUniprot,
                         SIGNAL(uniprotMappingFinished(int,QString)),
                         this,
                         SLOT(onUniprotMappingFinished(int,QString))
                         );
        this->processingInputFileAndWritingToTempFile();
        this->sleep(1);
        emit progressUpdated("Remove duplicated protein_id finished!\nStart query from Uniprot...");
        // Finished: Reading from input file and Writing to output file!
        // Next: proteinId -> query from uniprot -> uniprotKB

        this->mapUniprot->startRequestToQueryUniprot();
        this->exec();
    }
    else if (this->inputFileFormat == FileProcessingThread::INPUT_FILE_MSALIGN)
    {
        this->processingMsalignInputFileAndWritingToOutputFile();
        emit progressUpdated("Parse msalign file and writing to output file success!");
        this->sleep(1);
        emit progressUpdated("Thread exited. [Id: " +
                             QString::number(quintptr(QThread::currentThreadId())) + "]");
        this->quit();
        return;
    }
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

    // Fetch each line from the inputFile
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

        // Save fields useful to <outputFile>
        QString attributeProteinId; // We need protein_id only
        attributeProteinId = gtfFields.attributes.
                filter("protein_id",Qt::CaseInsensitive).join(',');

        loopCounts++;
        if(gtfFields.source != "HAVANA" || gtfFields.feature != "transcript"
                || attributeProteinId.isEmpty())
        {
            // <source> is not 'HAVANA' OR <feature> is not 'transcript'
            // OR <protein_id> Not Found, SKIP!
            // qDebug() << loopCounts << gtfFields.source << gtfFields.feature;
        }
        else
        {
            QString proteinIdExtracted =
                    this->extractProteinId(attributeProteinId).toString();

            mapUniprot->insertIntoProteinMap(proteinIdExtracted,QString());
            // Insert proteinId into map, preparing for query from uniprot

            stringBuffer = gtfFields.seqName + ',' + gtfFields.start
                    + ',' +gtfFields.end + ',' + gtfFields.strand + ',' + proteinIdExtracted;

            // 2019-05-30 Testing: observe the <gene_type> field
//            QRegularExpression re("^.*gene_type\\s\"(.*?)\".*;\n?");
//            QRegularExpressionMatch match = re.match(dataFields.at(8));
//            if(match.hasMatch())
//            {
//                // qDebug() << "match: " << match.captured(1) << match.captured(2);
//                stringBuffer += ',' + match.captured(1);
//            }

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

void FileProcessingThread::processingTempFileAndWritingToFinalOutputFile()
{
    // Open processed temp file for reading
    QFile tempOutputFile(this->temporaryFilePath);
    if( !tempOutputFile.open(QFile::ReadOnly) )
    {
        emit errorOccured("Unable to open tempFile");
        return;
    }
    else
    {
        emit progressUpdated("Open temp file <" + this->temporaryFilePath + "> Success!");
    }

    // Create output file
    QFile outputFile(this->outputFilePath);
    if( !outputFile.open(QFile::WriteOnly) )
    {
        emit errorOccured("Distination dir is read-only");
        return;
    }
    else
    {
        emit progressUpdated("Create output file <" + this->outputFilePath + "> Success!");
    }

    QTextStream tempOutputFileStream(&tempOutputFile);
    QTextStream outputFileStream(&outputFile);
    QString stringBuffer;
    qint32 loopCounts = 0;

    // Fetch each line from the tempFile
    while(tempOutputFileStream.readLineInto(&stringBuffer))
    {
        // Example lines:
        // chr1,65419,71585,ENSP00000493376
        // chr1,65419,65433,ENSP00000493376
        // chr1,65520,65573,ENSP00000493376
        QString proteinId =
                stringBuffer.section(',',-1,-1);

        QString uniprotReturnedValue =
                this->mapUniprot->getValueFromProteinMap(proteinId);

        stringBuffer.append(',' + uniprotReturnedValue);
        outputFileStream << stringBuffer + '\n';

        loopCounts++;
        if( 0 == loopCounts % 500)
        {
            emit progressUpdated("Processing the " +
                                 QString::number(loopCounts) +
                                 "th line: " + stringBuffer);
        }
    }

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

void FileProcessingThread::processingMsalignInputFileAndWritingToOutputFile()
{
    // Convert .msalign file
    QFile inputFile(this->inputFilePath);
    if( !inputFile.open(QFile::ReadOnly) )
    {
        emit errorOccured("Unable to open file");
        return;
    }
    else
    {
        emit progressUpdated("Open input file <" + this->inputFilePath + "> Success!");
    }

    QFile outputFile(this->outputFilePath);
    if( !outputFile.open(QFile::WriteOnly) )
    {
        emit errorOccured("Distination dir is read-only");
        return;
    }
    else
    {
        emit progressUpdated("Create output file <" + this->outputFilePath + "> Success!");
    }

    QTextStream inputFileStream(&inputFile);
    QTextStream outputFileStream(&outputFile);
    QString stringBuffer;
    qint32 loopCounts = 0;

    // Fetch each line from the inputFile
    while(inputFileStream.readLineInto(&stringBuffer))
    {
        loopCounts++;
        if(stringBuffer.isEmpty() || stringBuffer.at(0) == '#') // Comments, skip
        {
            continue;
        }

        if(stringBuffer.trimmed() == QString("BEGIN IONS"))
        {
            QString scanId;
            QString msScanMassAndPeakAundance;
            QRegularExpression regExpScanId("^SCANS=(\\d+)[\n\r]*");
            QRegularExpression regExpScanMassAndPeakAundance("^([\\d.]*?)\t([\\d.]*?)\t([\\d.]*?)[\r\n]*$");

            while (inputFileStream.readLineInto(&stringBuffer))
            {
                loopCounts++;
                if(stringBuffer.trimmed() == QString("END IONS"))
                {
                    stringBuffer = scanId + ',' +
                            msScanMassAndPeakAundance;

                    outputFileStream << stringBuffer << '\n';
                    break;
                }

                QRegularExpressionMatch match = regExpScanId.match(stringBuffer);
                if(match.hasMatch())
                {
                    scanId = match.captured(1);
                    continue;
                }

                match = regExpScanMassAndPeakAundance.match(stringBuffer);
                if(match.hasMatch())
                {
                    msScanMassAndPeakAundance += match.captured(1)
                            + '~' + match.captured(2) + ';';
                }
            }
        }

        if( 0 == loopCounts % 200)
        {
            emit progressUpdated("Processing the " +
                                 QString::number(loopCounts) +
                                 "th line: " + stringBuffer);
        }
    }
}
