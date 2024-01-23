// Класс для создания лог. файлов

#ifndef LOGMAKER_H
#define LOGMAKER_H

#include <QObject>
#include <QTextStream>
#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include "enum_list.h"

class LogMaker:public QFile
{
    Q_OBJECT
public:
    LogMaker(const QString log_file_path,
             bool fl_remove_old,           // флаг, перезаписывем ли уже имеющийся файл с таким же имененм
             QString  main_title,          // заголовок файла , записивается в начале файла
             QString format_time=" ",      // формат записи отметок времени, если "" => currentMSecsSinceEpoch()
             QString format_name=" ");     // формат имени файла, например "yyyy-mm-dd", если "" => имя файла с расшир уже есть в log_file_path
   ~LogMaker();

    bool     logOpen();
    void     logClose();
    bool     logWrite(int num, LogUseEnum use_flag, QString &data);


private:
    QString  m_log_path{""};   // путь к лог файлу
    bool     m_rem_old{false}; // флаг удаления старого лог файла
    QString  m_main_title{""}; // основной заголовок
    QString  m_format_time{""}; // формат времени, исп. в лог. записи
    QString  m_format_name{""}; // формат имени лог. файла

    bool     removeOld(const QString log_file_path);  // удаление старого файла
    bool     initLogPath();         // проверка и создание в случае необходимости папок для лог. файлов
    bool     checkDirExist(QString &dir_name_);   // проверка сущ. дир. для записи лога
    bool     makeDir(QString &dir_name_);         // создание дир. если ее нет


public slots:

   void      slotLogWrite(int num, LogUseEnum use_flag, QString &data);  // добавление записи в лог. файл

};

#endif // LOGMAKER_H
