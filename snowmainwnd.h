#ifndef SNOWMAINWND_H
#define SNOWMAINWND_H

#include <QDialog>
#include "fileprocessingthread.h"

namespace Ui {
class SnowMainWnd;
}

class SnowMainWnd : public QDialog
{
    Q_OBJECT

public:
    explicit SnowMainWnd(QWidget *parent = 0);
    ~SnowMainWnd();

private slots:
    void onStartButtonClicked();

    void on_toolButton_InputFileChoose_clicked();
    void on_toolButton_OutputFileChoose_clicked();

    void onFileThreadProgressUpdated(QString newLine);
    void onFileThreadErrorOccured(QString errorReason);
    void onFileThreadFinished();

private:
    Ui::SnowMainWnd *ui;
    FileProcessingThread *fileThread;
};

#endif // SNOWMAINWND_H
