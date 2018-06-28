#include "reportform.h"
#include "ui_reportform.h"

#include <QFile>

ReportForm::ReportForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportForm)
{
    ui->setupUi(this);
}

ReportForm::~ReportForm()
{
    delete ui;
}

void ReportForm::setContents(QString filename)
{
    QByteArray contents ;
    QFile file(filename) ;
    if (file.exists()  && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        contents=file.readAll() ;
        file.close() ;
    }
    ui->plainTextEdit->document()->setPlainText(QString::fromLatin1(contents)) ;
}
