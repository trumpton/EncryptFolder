#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../Lib/encryption.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_folder_pushButton_clicked();
    void on_start_pushButton_clicked();
    void on_actionLogout_triggered();
    void on_actionSetKey_triggered();
    void on_actionLogin_triggered();
    void on_ciphertext_lineEdit_textChanged(const QString &arg1);

    void on_plaintext_lineEdit_textChanged(const QString &arg1);

    void on_actionAbout_triggered();

private:
    bool doabort ;
    Encryption *enc ;
    Ui::MainWindow *ui;

private:
    void log(QString message, bool initialise=false) ;
    void refreshMenu() ;
    bool fileNameMatch(QString f1, QString f2) ;
    bool processFolder(QString folder, QString ptext, QString ctext, bool hierarchical, bool encrypt, bool isroot) ;
    bool loadEnc(QString filename, QByteArray& contents) ;
    bool saveEnc(QString filename, QByteArray contents) ;
    bool savePlain(QString filename, QByteArray contents) ;
    bool loadPlain(QString filename, QByteArray& contents) ;


};

#endif // MAINWINDOW_H
