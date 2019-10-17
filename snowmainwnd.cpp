#include "snowmainwnd.h"
#include "fileprocessingthread.h"
#include "ui_snowmainwnd.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QElapsedTimer>
#include <QDebug>

SnowMainWnd::SnowMainWnd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SnowMainWnd)
{
    ui->setupUi(this);
    this->fileThread = nullptr;
    this->elapsedTimer = new QElapsedTimer();

    QStringList fileTypeList;
    // fileTypeList << "Gtf (Genome Annotation)" << "MsAlign (Alignment Results)";
    fileTypeList.insert(FileProcessingThread::INPUT_FILE_GTF, "Gtf (Genome Annotation)");
    fileTypeList.insert(FileProcessingThread::INPUT_FILE_MSALIGN, "MsAlign (Deconvoluted Spectra)");
    this->ui->comboBox_FileType->addItems(fileTypeList);

    QObject::connect(this->ui->pushButton_Start,SIGNAL(clicked(bool)),
                     this,SLOT(onStartButtonClicked()));
}

SnowMainWnd::~SnowMainWnd()
{
    delete ui;
}

void SnowMainWnd::onStartButtonClicked()
{
    this->fileThread = new FileProcessingThread(
                this->ui->comboBox_FileType->currentIndex(),
                this->ui->lineEdit_InputFilePath->text(),
                this->ui->lineEdit_OutputFilePath->text()
                );

    QObject::connect(this->fileThread,SIGNAL(progressUpdated(QString)),
                     this,SLOT(onFileThreadProgressUpdated(QString)));
    QObject::connect(this->fileThread,SIGNAL(errorOccured(QString)),
                     this,SLOT(onFileThreadErrorOccured(QString)));
    QObject::connect(this->fileThread,SIGNAL(finished()),
                     this,SLOT(onFileThreadFinished()));

    this->ui->textEdit_Info->clear();
    this->ui->pushButton_Start->setDisabled(true);
    this->fileThread->start();

    if(this->elapsedTimer->isValid())
    {
        this->elapsedTimer->restart();
    }
    else
    {
        this->elapsedTimer->start();
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

void SnowMainWnd::onFileThreadProgressUpdated(QString newLine)
{
    this->ui->textEdit_Info->append(newLine);
    this->ui->textEdit_Info->verticalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);
}

void SnowMainWnd::onFileThreadErrorOccured(QString errorReason)
{
    QMessageBox::critical(this,"Error",errorReason);
    this->ui->pushButton_Start->setDisabled(false);
}

void SnowMainWnd::onFileThreadFinished()
{
    this->ui->pushButton_Start->setDisabled(false);
    this->fileThread->deleteLater();
    this->fileThread = nullptr;

    this->onFileThreadProgressUpdated("Finished! Elapsed Time: " +
                    QString::number(this->elapsedTimer->elapsed() / 1000) +
                    "s");
}
