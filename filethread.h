#ifndef FILETHREAD_H
#define FILETHREAD_H

#include <QThread>
#include <QMutex>
#include <QMap>
#include <QTime>

class QFile;

class FileThread : public QThread
{
    Q_OBJECT

public:
    FileThread();
    ~FileThread();
    void hashDir(QString name);
    void findDir(QString name);
    QString state();

public slots:
    void setOutFile(QString name) { outFile = name; }
    void setDublicate(bool f) { dublicate=f; }
    void setSleeped(bool f) { sleeped = f; }
    void setToTrash(bool f) { toTrash = f; }
    void setSave(bool f) { toSave = f; }
    void setBreak() { breaked = true; }

signals:
    void done();

private:
    void run();
    void runHash(bool toRemove=false);
    void quickRemove();
    void addRecursive(QString dirName, QStringList &files);
    void readHashMap();
    void remove(QString name);
    void save();
    QByteArray readHash(QString name);

    bool started=true;
    bool breaked=false;
    bool dublicate=true;
    bool sleeped=false;
    bool toTrash=true;
    bool toSave=false;
    int stepFiles = 0;
    int stepHash = 0;
    int stepRemove = 0;
    quint64 sizeRemove = 0;

    enum { NOP, SCAN, DEL, QUICK };
    int oper = NOP;     // операция 1-скан, 2-удаление дубликатов 3-быстрое удаление

    QStringList files;
    QMutex mu;
    QString dir;        // директория сканирования
    QString outFile = "out.sha";
    QMap<QByteArray,quint64> map;
    QTime time;
    int dt = 0;
};

#endif // FILETHREAD_H
