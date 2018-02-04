#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FileThread;
class QSettings;

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
    void timerEvent(QTimerEvent *e);
    void on_pbSelectFile_clicked();
    void on_pbSelectDir_clicked();
    void on_pbHash_clicked();
    void on_pbRemove_clicked();

private:
    void block(bool blocked);

    Ui::MainWindow *ui;
    FileThread *filethread;
    QSettings *settings;
};

#endif // MAINWINDOW_H
