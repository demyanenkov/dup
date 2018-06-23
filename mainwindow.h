#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FileThread;
class QSettings;

namespace Ui {
class MainWindow;
}

class QListWidgetItem;

struct Data
{
    QString file;
    QString dir;
    QListWidgetItem *item;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void timerEvent(QTimerEvent *e);
    void on_pbSelectFile_clicked();
    void on_pbSelectDir_clicked();
    void on_pbHash_clicked();
    void on_pbRemove_clicked();
    void on_checkBoxSetup_toggled(bool checked);
    void on_pushButtonAdd_clicked();
    void on_pushButtonRemove_clicked();
    void on_listWidget_currentRowChanged(int row);
    void on_comboBoxFile_currentTextChanged(const QString &text);
    void on_comboBoxDir_currentTextChanged(const QString &text);

private:
    void block(bool blocked);
    void addItem(QString name="...", QString file="default.sha", QString dir = ".");
    void removeItem(int n);

    Ui::MainWindow *ui;
    FileThread *filethread;
    QSettings *settings;
    QList<Data> list;
    int index = -1;
};

#endif // MAINWINDOW_H
