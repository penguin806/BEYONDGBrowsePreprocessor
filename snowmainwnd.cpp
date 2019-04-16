#include "snowmainwnd.h"
#include "ui_snowmainwnd.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QDebug>

SnowMainWnd::SnowMainWnd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SnowMainWnd)
{
    ui->setupUi(this);
    QObject::connect(this->ui->pushButton_Start,SIGNAL(clicked(bool)),
                     this,SLOT(onStartButtonClicked()));
}

SnowMainWnd::~SnowMainWnd()
{
    delete ui;
}

void SnowMainWnd::onStartButtonClicked()
{
    QFile inputFile(this->ui->lineEdit_InputFilePath->text());
    if( !inputFile.open(QFile::ReadOnly) )
    {
        QMessageBox::critical(this, "Error", "Unable to open file");
        return;
    }

    QFile outputFile(this->ui->lineEdit_OutputFilePath->text());
    if( !outputFile.open(QFile::WriteOnly) )
    {
        QMessageBox::critical(this, "Error", "Distination dir is read-only");
        return;
    }

    QTextStream inputFileStream(&inputFile);
    QTextStream outputFileStream(&outputFile);
    QString stringBuffer;

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
            QMessageBox::critical(this,"Error","Not a valid gtf file");
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
        stringBuffer = gtfFields.seqName + ',' + gtfFields.start
                + ',' +gtfFields.end + ',' + gtfFields.attributes.
                filter("gene_id",Qt::CaseInsensitive).join(',');

        this->ui->textEdit_Info->append(stringBuffer);
        this->ui->textEdit_Info->verticalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);
        outputFileStream << stringBuffer << '\n';
    }

}

void SnowMainWnd::on_toolButton_InputFileChoose_clicked()
{
    QString inputFilePath;
    inputFilePath = QFileDialog::getOpenFileName(this, "Select Input File");
    this->ui->lineEdit_InputFilePath->setText(inputFilePath);
}

void SnowMainWnd::on_toolButton_OutputFileChoose_clicked()
{
    QString outputFilePath;
    outputFilePath = QFileDialog::getSaveFileName(this, "Save as");
    this->ui->lineEdit_OutputFilePath->setText(outputFilePath);
}
