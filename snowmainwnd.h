#ifndef SNOWMAINWND_H
#define SNOWMAINWND_H

#include <QDialog>

namespace Ui {
class SnowMainWnd;
}

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

private:
    Ui::SnowMainWnd *ui;
};

#endif // SNOWMAINWND_H
