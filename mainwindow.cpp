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

    settings->beginGroup("General");
    settings->endGroup();

    for(int i=0; ;i++)
    {
        settings->beginGroup(QString("Set%1").arg(i));

        QString name = "...";
        if(!settings->value("Name").isValid()) {
            settings->endGroup();
            break;
        }
        name = settings->value("Name").toString();

        Data data { "default.sha", ".", nullptr};
        if(settings->value("File").isValid()) data.file = settings->value("File").toString();
        if(settings->value("Dir").isValid()) data.dir = settings->value("Dir").toString();
        addItem(name, data.file, data.dir);

        settings->endGroup();
    }

    startTimer(100);
}

MainWindow::~MainWindow()
{
    settings->clear();

    settings->beginGroup("General");
    settings->endGroup();

    for(int i=0; i<ui->listWidget->count(); i++)
    {
        settings->beginGroup(QString("Set%1").arg(i));

        settings->setValue("Name", ui->listWidget->item(i)->text());
        settings->setValue("File",list[i].file);
        settings->setValue("Dir", list[i].dir);

        settings->endGroup();
    }

    delete filethread;
    delete ui;
}

void MainWindow::on_pbSelectFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", "*.sha;;*.*", nullptr, QFileDialog::DontConfirmOverwrite);

    if(fileName.isEmpty()) return;

    if(QFileInfo(fileName).absolutePath() == QDir::currentPath()) fileName = QFileInfo(fileName).fileName();

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

void MainWindow::on_checkBoxSetup_toggled(bool checked)
{
    ui->groupBoxSetup->setVisible(checked);
}

void MainWindow::on_pushButtonAdd_clicked()
{
    addItem();
}

void MainWindow::on_pushButtonRemove_clicked()
{
    removeItem(ui->listWidget->currentRow());
}

void MainWindow::addItem(QString name, QString file, QString dir)
{
    // block on
    index = -1;

    QListWidgetItem *item = new QListWidgetItem(name);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    Data data { file, dir, item };
    list.append(data);
    ui->listWidget->addItem(item);
    ui->comboBoxDir->setCurrentText(dir);
    ui->comboBoxFile->setCurrentText(file);
    ui->listWidget->setCurrentRow(ui->listWidget->count()-1);

    // block off
    index = ui->listWidget->count()-1;

}

void MainWindow::removeItem(int n)
{
    if(ui->listWidget->count() < 1 || n < 0) return;

    // block on
    index = -1;
    int row = ui->listWidget->currentRow();
    ui->listWidget->setCurrentRow(-1);

    ui->listWidget->removeItemWidget(ui->listWidget->item(row));
    delete ui->listWidget->item(row);
    list.removeAt(n);

    // block off
    index = 0;
    ui->listWidget->setCurrentRow(0);
}

void MainWindow::on_listWidget_currentRowChanged(int row)
{
    if(row < 0 || index < 0) return;
    index = row;
    ui->comboBoxDir->setCurrentText(list[row].dir);
    ui->comboBoxFile->setCurrentText(list[row].file);
}

void MainWindow::on_comboBoxFile_currentTextChanged(const QString &text)
{
    if(index < 0) return;
    list[index].file = text;
}

void MainWindow::on_comboBoxDir_currentTextChanged(const QString &text)
{
    if(index < 0) return;
    list[index].dir = text;
}
