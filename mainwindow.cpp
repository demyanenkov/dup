#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filethread.h"

#include <QFileDialog>
#include <QCoreApplication>
#include <QSettings>

#include <QDate>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    filethread = new FileThread;
    setWindowTitle(QString("DupRemover   ") + QString(__DATE__) + QString("   ") + QString(__TIME__) + QString("   Qt") + QT_VERSION_STR);

    connect(ui->pbSleep, SIGNAL(clicked(bool)), filethread, SLOT(setSleeped(bool)));
    connect(ui->pbBreak, SIGNAL(clicked(bool)), filethread, SLOT(setBreak()));
    connect(ui->checkBoxRemove, SIGNAL(toggled(bool)), filethread, SLOT(setDublicate(bool)));
    connect(ui->checkBoxTrash, SIGNAL(toggled(bool)), filethread, SLOT(setToTrash(bool)));
    connect(ui->checkBoxSave, SIGNAL(toggled(bool)), filethread, SLOT(setSave(bool)));
    connect(ui->comboBoxFile, SIGNAL(currentTextChanged(QString)), filethread, SLOT(setOutFile(QString)));
    connect(filethread, FileThread::done, [&](){block(false);});

    settings = new QSettings("dup.ini", QSettings::IniFormat, this);
    settings->setIniCodec("UTF-8");
    if(settings->value("File").isValid()) ui->comboBoxFile->setCurrentText(settings->value("File").toString());
    if(settings->value("Dir").isValid()) ui->comboBoxDir->setCurrentText(settings->value("Dir").toString());

    startTimer(100);
}

MainWindow::~MainWindow()
{
    settings->setValue("File",ui->comboBoxFile->currentText());
    settings->setValue("Dir",ui->comboBoxDir->currentText());

    delete filethread;
    delete ui;
}

void MainWindow::on_pbSelectFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", "*.sha;;*.*");

    if(fileName.isEmpty()) return;

    if(ui->comboBoxFile->findText(fileName)<1) ui->comboBoxFile->insertItem(0,fileName);
    ui->comboBoxFile->setCurrentText(fileName);
}

void MainWindow::on_pbSelectDir_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Select directory"), "");

    if(dirName.isEmpty()) return;

    if(ui->comboBoxDir->findText(dirName)<1) ui->comboBoxDir->insertItem(0,dirName);
    ui->comboBoxDir->setCurrentText(dirName);
}

void MainWindow::on_pbHash_clicked()
{
    block(true);
    filethread->hashDir(ui->comboBoxDir->currentText());
}

void MainWindow::on_pbRemove_clicked()
{
    block(true);
    filethread->findDir(ui->comboBoxDir->currentText());
}

void MainWindow::timerEvent(QTimerEvent *)
{
    ui->lProgress->setText(filethread->state());
}

void MainWindow::block(bool blocked)
{
    ui->pbHash->setEnabled(!blocked);
    ui->pbRemove->setEnabled(!blocked);
    ui->groupBoxSetup->setEnabled(!blocked);
}
