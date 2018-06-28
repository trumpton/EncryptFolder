#ifndef REPORTFORM_H
#define REPORTFORM_H

#include <QDialog>

namespace Ui {
class ReportForm;
}

class ReportForm : public QDialog
{
    Q_OBJECT

public:
    explicit ReportForm(QWidget *parent = 0);
    void setContents(QString filename) ;
    ~ReportForm();

private:
    Ui::ReportForm *ui;
};

#endif // REPORTFORM_H
