#include "filethread.h"
#include <QCryptographicHash>
#include <QFileDialog>
#include <QMutexLocker>
#include <windows.h>
#include <QDebug>

FileThread::FileThread()
{
    start();
}

FileThread::~FileThread()
{
    started = false;
    breaked = true;
    sleeped = false;
    msleep(100);
}

void FileThread::run()
{
    while(started){
        if(oper!=NOP){
            files.clear();
            time = QTime().currentTime();
            readHashMap();
            stepRemove = sizeRemove = 0;

            // чтение дерева каталогов
            addRecursive(dir, files);
            dir.clear();

            if(oper==SCAN) runHash();
            else if(oper==DEL) runHash(true);
            else if(oper==QUICK) quickRemove();

            if(!breaked && oper!=QUICK) save();
            readHashMap();

            stepFiles = 0;
            emit done();
            time=QTime();

            oper=NOP;
        }
        breaked=false;
        msleep(10);
    }
}

void FileThread::hashDir(QString name)
{
    if(QFileInfo(name).isDir()) dir=name, oper=SCAN;
    else done();
}

void FileThread::findDir(QString name)
{
    if(QFileInfo(name).isDir()) dir=name, oper=toSave?DEL:QUICK;
    else done();
}

void FileThread::addRecursive(QString dirName, QStringList &files)
{
    QDir dir(dirName);

    QStringList list = dir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Dirs | QDir::Files);

    for(QString &name:list){
        if(breaked) break;
        while(sleeped) msleep(1);

        name = dirName+"/"+name;
        if(QFileInfo(name).isDir()) addRecursive(name, files);
        else if(QFileInfo(name).isFile()){
            QMutexLocker lock(&mu);
            files+=name, stepFiles++;
        }
    }
}

void FileThread::runHash(bool toRemove)
{
    // расчет хэша
    for(QString fileName:files){
        if(breaked) break;
        while(sleeped) msleep(1);

        QByteArray hash = readHash(fileName);
        if(hash.isEmpty()) continue;
        stepHash ++;

        if(toRemove && map.contains(hash)) // Удаление дубликатов из известных
            remove(fileName);
        else if(!toRemove || dublicate){
            QMutexLocker lock(&mu);
            map.insert(hash,QFile(fileName).size()); // Для удаления повторов
        }
    }
}

void FileThread::quickRemove()
{
    // Чтение фала с хэшем
    struct Val { QByteArray hash; QString name; }; // Поля для сортировки по размеру
    QMap<quint64, Val> map;
    for(auto v=this->map.begin(); v!=this->map.end(); v++)
        map.insertMulti(v.value(), Val{v.key(),""});

    // Поиск
    for(QString name:files){
        if(breaked) break;
        while(sleeped) msleep(1);
        stepHash++;

        // поиск файла такого размера
        quint64 size = QFile(name).size();
        QByteArray hash;
        if(map.contains(size)){
            //чтение хэша
            hash = readHash(name);
            if(hash.isEmpty()) continue;

            auto i = map.find(size);
            while (i != map.end() && i.key() == size) {
                if(i.value().hash.isEmpty())
                    i.value().hash = readHash(i.value().name);

                if(hash==i.value().hash) {
                    size=0;
                    remove(name);
                    break;
                }
                ++i;
            }
        }

        if(size) {
            QMutexLocker lock(&mu);
            map.insertMulti(size, Val {hash,name});
        }
    }
}

QByteArray FileThread::readHash(QString name)
{
    QByteArray hash;

    QFile file(name);
    if(file.exists() && file.open(QIODevice::ReadOnly)){
        QByteArray ba = file.readAll();
        file.close();
        if(!ba.isEmpty())
            hash = QCryptographicHash::hash(ba,QCryptographicHash::Sha1);
    }
    return hash;
}

void FileThread::save()
{
    QString buf;

    // Список для сохранения
    for(auto v=map.begin(); v!=map.end(); v++){
        if(breaked) break;
        while(sleeped) msleep(1);

        for(quint8 b:v.key()) buf+=QString().sprintf("%02x",int(b));
        buf += QString().sprintf(" %llu\n", (quint64)v.value());
    }

    QFile out(outFile);

    // Сохранение резервной копии
    if(out.exists() && out.size() && out.size()!=buf.size()){
        QString name = QFileInfo(out.fileName()).absoluteDir().path()
                + "/" + "~" + QFileInfo(out.fileName()).fileName();
        QFile(name).remove();
        out.copy(name);
    }

    out.open(QIODevice::WriteOnly);
    out.write(buf.toLocal8Bit());
    out.close();
}

void FileThread::readHashMap()
{
    mu.lock();
    map.clear();
    mu.unlock();

    stepHash = 0;

    QFile file(outFile);

    if(!file.open(QIODevice::ReadOnly)) return;

    QByteArray ba;

    while(!(ba=file.readLine()).isEmpty()){
        quint64 size = ba.split(' ')[1].replace('\n',"").toULongLong();
        ba=ba.split(' ')[0];

        QByteArray hash;

        while(ba.size()>0){
            hash+=QByteArray().fromHex(ba.mid(0,2));
            ba.remove(0,2);
        }

        QMutexLocker lock(&mu);
        map.insert(hash,size);
    }

    file.close();
}

QString FileThread::state()
{
    QString s,t;
    QMutexLocker lock(&mu);

    if(!time.isNull()) dt = time.secsTo(QTime::currentTime());

    s+=t.sprintf("%02i:%02i:%02i  [%i]  %i/%i",dt/3600, (dt/60)%60, dt%60, map.size(), stepHash, stepFiles);
    if(stepRemove) s+=t.sprintf(" remove %i %llu(%lluM)",stepRemove, sizeRemove, sizeRemove/1048576);

    return s;
}

void FileThread::remove(QString name)
{
    QFile file(name);
    sizeRemove += QFile(name).size();
    stepRemove++;
    if(!toTrash){ file.remove(); return; }// Полное удаление

    auto path = QDir::toNativeSeparators(file.fileName()).toStdWString();
    int l = path.length()*2;
    wchar_t pc[l+4]; // два завершающих wchar_t нуля !!!
    memset(pc, 0, l+4);
    memcpy(pc, path.data(), l);

    SHFILEOPSTRUCTW shfop = { 0, FO_DELETE, pc, 0, FOF_ALLOWUNDO | FOF_NO_UI, 0, 0, 0 };
    SHFileOperationW(&shfop);
}
