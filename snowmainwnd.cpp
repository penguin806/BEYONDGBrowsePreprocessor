#include "snowmainwnd.h"
#include "ui_snowmainwnd.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
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
    QFile inputFile("D:\\top20line.txt");
    if( !inputFile.open(QFile::ReadOnly) )
    {
        QMessageBox::critical(this, "Error", "Unable to open file");
        return;
    }

    QTextStream inputFileStream(&inputFile);
    QString stringBuffer;
    while(inputFileStream.readLineInto(&stringBuffer))
    {
        if(stringBuffer.at(0) == '#') // Comments
        {
            continue;
        }
        this->ui->textEdit_Info->append(stringBuffer);

        QStringList dataFields;
        dataFields = stringBuffer.split('\t');
        if(dataFields.length() < 9)
        {
            QMessageBox::critical(this,"Error","Not a valid gtf file");
            return;
        }

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
