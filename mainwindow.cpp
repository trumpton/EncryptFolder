#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

#include "version.h"
#include "reportform.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0) ;
    enc = new Encryption(QString("trumpton.uk"), QString("TextFileEncryption")) ;
    refreshMenu() ;
}


MainWindow::~MainWindow()
{
    delete enc ;
    delete ui;
}

void MainWindow::on_folder_pushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    ui->folder_lineEdit->setText(dir) ;
    refreshMenu() ;
}

void MainWindow::on_start_pushButton_clicked()
{
    QString ptext = ui->plaintext_lineEdit->text().replace(".","") ;
    QString ctext = ui->ciphertext_lineEdit->text().replace(".","") ;
    QString path = ui->folder_lineEdit->text() ;
    bool encrypt = ui->encrypt_radioButton->isChecked() ;
    bool hierarchical = ui->hierarchy_checkBox->isChecked() ;
    //
    // Check Path
    //

    if (path.isEmpty()) {
        // Error, no path selected
    }

    //
    // Check Extensions
    //

    if (fileNameMatch(ptext, ctext)) {
        // Error, extensions are the same
    }


    if (!enc->loggedIn()) {
        enc->login() ;
    }

    if (enc->loggedIn()) {
        bool overwrite = ui->overwrite_checkBox->isChecked() ;
        log(QString(encrypt?"Encrypting ...\n":"Decrypting ...\n"), true) ;
        if (!processFolder(path, ptext, ctext, hierarchical, encrypt, overwrite, true)) {
            // Error processing data
        }

    }

    ReportForm form ;
    form.setContents(path + "/encryptfolder.log") ;
    form.exec() ;
}

bool MainWindow::fileNameMatch(QString f1, QString f2)
{
#ifdef WIN32
    return (f1.tolower().compare(f2.tolower())==0) ;
#else
    return (f1.compare(f2)==0) ;
#endif
}

bool MainWindow::processFolder(QString folder, QString ptext, QString ctext, bool hierarchical, bool encrypt, bool overwrite, bool isroot)
{
    QDir dir(folder);
    if (!dir.exists()) {
        log(QString("Error, folder does not exist: ") + folder + QString("\n")) ;
        return false;
    }

    if (isroot) {
        doabort=false ;
        ui->progressBar->setMaximum(dir.entryList(QDir::Files).size() +
                              dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() );
    }

    ui->statusBar->showMessage("Processing ....") ;
    qApp->processEvents() ;

    int index=0 ;

    if (hierarchical) {
        QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot) ;
        foreach (QString d, dirs) {
            QString sub_path = folder + QDir::separator() + d ;
            if (doabort) {
                return false ;
            }
            if (!processFolder(sub_path, ptext, ctext, hierarchical, encrypt, overwrite, false)) {
                return false ;
            }
            if (isroot) ui->progressBar->setValue(++index) ;
        }
    }

    log(QString("\nProcessing folder: ") + folder + QString(".\n")) ;

    QStringList files = dir.entryList(QDir::Files) ;
    foreach (QString f, files) {
        QString file_path = folder + QDir::separator() + f ;
        QFileInfo file(file_path) ;

        ui->statusBar->showMessage(f) ;
        qApp->processEvents() ;
        if (doabort) {
            return false ;
        }

        QString file_base = folder + QDir::separator() + file.completeBaseName() ;
        QString file_ext = file.completeSuffix() ;

        if (encrypt) {
            if (fileNameMatch(file_ext, ptext)) {
                // Do Encryption
                QByteArray data ;
                log(QString("    Encrypting: ") + file_base + QString(".") + ptext) ;
                if (loadPlain(file_base + QString(".") + ptext, data)) {
                    ui->statusBar->showMessage(file_base) ;
                    switch (saveEnc(file_base + QString(".") + ctext, data, overwrite)) {
                    case SaveOk:
                        log(QString(": OK\n")) ;
                        break ;
                    case SaveFailed:
                        log(QString(": ERROR failed to save\n")) ;
                        break ;
                    case FileExists:
                        log(QString(": WARNING skipped as file exists\n")) ;
                        break ;
                    }
                } else {
                    log(QString(": ERROR failed to load\n")) ;
                }
            }
        } else {
            if (fileNameMatch(file_ext, ctext)) {
                QByteArray data ;
                log(QString("    Decrypting: ") + file_base + QString(".") + ctext) ;
                if (loadEnc(file_base + QString(".") + ctext, data)) {
                    ui->statusBar->showMessage(file_base) ;
                    switch (savePlain(file_base + QString(".") + ptext, data, overwrite)) {
                    case SaveOk:
                        log(QString(": OK\n")) ;
                        break ;
                    case SaveFailed:
                        log(QString(": ERROR failed to save\n")) ;
                        break ;
                    case FileExists:
                        log(QString(": WARNING skipped as file exists\n")) ;
                        break ;
                    }
                } else {
                    log(QString(": ERROR failed to load\n")) ;
                }
            }
        }

        if (isroot) ui->progressBar->setValue(++index) ;
        qApp->processEvents() ;
    }

    if (isroot) {
        ui->statusBar->showMessage("Complete", 10);
    }

    return true ;
}

void MainWindow::refreshMenu()
{
    if (enc->loggedIn()) {
           ui->actionLogin->setEnabled(false) ;
           ui->actionSetKey->setEnabled(false) ;
           ui->actionLogout->setEnabled(true);
    } else {
        ui->actionLogin->setEnabled(true) ;
        ui->actionSetKey->setEnabled(true) ;
        ui->actionLogout->setEnabled(false);
    }
    if (ui->folder_lineEdit->text().isEmpty() || ui->ciphertext_lineEdit->text().isEmpty() || ui->plaintext_lineEdit->text().isEmpty()) {
        ui->progressBar->setEnabled(false) ;
        ui->start_pushButton->setEnabled(false) ;
    } else {
        ui->progressBar->setEnabled(true) ;
        ui->start_pushButton->setEnabled(true) ;
    }
}


bool MainWindow::loadEnc(QString filename, QByteArray& contents)
{
    QFile file(filename) ;
    if (!file.exists()) return false ;
    if (!enc->load(filename, contents)) {
        return false ;
    } else {
        return true ;
    }
}

SaveStatus MainWindow::saveEnc(QString filename, QByteArray contents, bool overwrite)
{
    QFile file(filename) ;
    if (file.exists() && !overwrite) return FileExists ;
    if (!enc->save(filename, contents)) {
        return SaveFailed ;
    } else {
        return SaveOk ;
    }
}

bool MainWindow::loadPlain(QString filename, QByteArray& contents)
{
    QFile file(filename) ;
    if (!file.exists()) return false ;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false ;
    } else {
        contents=file.readAll() ;
        if (file.error()) {
            file.close() ;
            return false ;
        } else {
            file.close() ;
            return true ;
        }
    }

}

SaveStatus MainWindow::savePlain(QString filename, QByteArray contents, bool overwrite)
{
    QFile file(filename) ;
    if (file.exists() && !overwrite) return FileExists ;
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return SaveFailed;
    QTextStream out(&file);
    out << contents ;
    if (file.flush()) {
        file.close() ;
        return SaveOk ;
    } else {
        file.close() ;
        return SaveFailed ;
    }
}

void MainWindow::on_actionLogout_triggered()
{
    enc->logout() ;
    refreshMenu() ;
}

void MainWindow::on_actionSetKey_triggered()
{
    enc->setKey();
    refreshMenu() ;
}

void MainWindow::on_actionLogin_triggered()
{
    enc->login() ;
    refreshMenu() ;
}

void MainWindow::on_ciphertext_lineEdit_textChanged(const QString &arg1)
{
    refreshMenu() ;
}

void MainWindow::on_plaintext_lineEdit_textChanged(const QString &arg1)
{
    refreshMenu() ;
}

void MainWindow::log(QString message, bool initialise)
{
    QString folder = ui->folder_lineEdit->text() ;
    if (folder.isEmpty()) return ;
    QFile file(folder + "/encryptfolder.log") ;
    if (!file.open(initialise?QIODevice::WriteOnly:QIODevice::Append)) return ;
    file.write(message.toLatin1()) ;
    file.close() ;
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::warning(this, QString("About Encrypt Folder"), QString("This is version " BUILDVERSION " (" BUILDDATE ") of Encrypt Folder, and was built on   " COMPILEDATE), QMessageBox::Ok) ;
}
