# 1. Составление хэша файлов в папке (рекурсивно)
# 2. Удаление файлов из папки (рекурсивно) если есть в известном хэше

# 2017-09-05 Рабочий вариант Qt5.7.0 Windows XP SP3
# 2017-09-06 Удаление в карзину под виндами
# 2017-09-13 bugfix быстрое удаление дублей (insertMulti)
# 2018-06-23 Список настроек

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = dup
TEMPLATE = app
VERSION  = 0.3

SOURCES += main.cpp mainwindow.cpp filethread.cpp
HEADERS += mainwindow.h filethread.h
FORMS   += mainwindow.ui

DISTFILES += README.md dup.rc
RC_FILE = dup.rc
