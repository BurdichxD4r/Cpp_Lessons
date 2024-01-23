// Класс для создания лог файлов
#include "logmaker.h"
#include <QDir>
//-------------------------------------------------
// log_file_path - путь к файлу и имя файла с расширением относительно папки ../LOG/APP_NAME
LogMaker::LogMaker(const QString log_file_path, bool fl_remove_old,        // флаг, перезаписывем ли уже имеющийся файл с таким же имененм
                   QString  main_title,            // заголовок файла , записивается в начале файла
                   QString format_time,      // формат записи отметок времени, если "" => currentMSecsSinceEpoch()
                   QString format_name):
    m_rem_old(fl_remove_old), m_main_title(main_title), m_format_time(format_time),m_format_name(format_name)
{
    QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
    QString APP_NAME = "/"+QCoreApplication::applicationName() + "/";
    QString LOG_PATH  = "/LOG/";               // дир. с логами.
    QString fl_path = APP_ABS_PATH + LOG_PATH + APP_NAME + log_file_path;

    if(initLogPath()){    // создаем если необходимо дир. для лог. файлов

      if (m_format_name != " ") { // если формат имени файла задан польз.
        QString s = QDateTime::currentDateTime().toLocalTime().toString("yy_MM_dd_hh_mm");
        QString LOG_FL_NAME = s + ".log";
        fl_path = fl_path + LOG_FL_NAME;
      }
      else{   // если формат имени не задан, скорее всего необходимо уничтожить уже имеющ. файл с таким же имененм
        if (m_rem_old){
           removeOld(fl_path); // удаляем старый файл
        }
    }
    setFileName(fl_path);
    }
}
//----------------------------------------------------
LogMaker::~LogMaker()
{

}
//--------------------------------------------------------------------
// проверка наличие и создание дир-рий лог файлов
bool LogMaker::initLogPath()
{
    bool res{false};
    QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
    QString APP_ABS_NAME = QCoreApplication::applicationName();
    QString LOG_PATH  = APP_ABS_PATH + "/LOG/";               // общая дир. лог файлов

    QString LOG_FL_DIR = LOG_PATH + APP_ABS_NAME + "/";       // поддиректория с именем приложения для лог файлов

    if (!checkDirExist(LOG_PATH))     // если LOG дир. не существ.
          if(!makeDir(LOG_PATH))
              {}// пытаемся ее создать
           //  {res = false;}
         // else
           //   res = true;
   // else
     //      res = true;

    if (!checkDirExist(LOG_FL_DIR))     // если дир. не существ.
          if(!makeDir(LOG_FL_DIR))      // пытаемся ее создать
             {res = false;}
          else
              res = true;
    else
           res = true;

    return res;
 }
//-------------------------------------------------------------------
// проверка существования директории с заданным имененм
bool LogMaker:: checkDirExist(QString &dir_name_)
{
    QDir dir(dir_name_);
    if (!dir.exists()){
     //   QMessageBox::critical(0,"Ошибка","Директория:  "+
     //                         dir_name_+
     //                         "  НЕ существует. Мы попробуем её создать." );
        return false;
    }
    else
        return true;
}
//---------------------------------------------------------------------
// создание директории с заданным имененм
bool LogMaker:: makeDir(QString &dir_name_)
{
    QDir dir(dir_name_);
    if (!dir.mkdir(dir_name_)){
        //QMessageBox::critical(0,"Ошибка","Директория:  "+
       //                       dir_name_+
      //                        "  НЕ может быть создана.Проверте права доступа." );
        return false;
    }
    else
        return true;
}
//--------------------------------------------------------------------
// открываем файл для работы (один раз в начале сеанса работы)
bool  LogMaker::logOpen()
{
    bool res{false};

    if(exists()){
       if (open((QIODevice::Append)|(QIODevice::Text)|(QIODevice::WriteOnly))){
          res = true;
        }
       else
          res = false;
    }
    if (!exists()){
        if(!open(QIODevice::WriteOnly)/*|(QIODevice::Append)*/){
          res = false;
        }
        else
          res = true;
    }
    return res;
}
//-----------------------------------------------------------------------------------------------
void LogMaker::logClose()
{
    if (isOpen()){
        close();
    }
}
//-----------------------------------------------------------------------------------------------
bool LogMaker::logWrite(int num, LogUseEnum use_flag, QString &data)
{
    bool res{false};
    QString use_flag_str{"UNKN"};

    switch (use_flag){
      case TRANSP_TCP : use_flag_str = "TR_TCP"; break;     // сообщ. принято/отправлено с транспорт. уровня(tcp)
      case TRANSP_UDP : use_flag_str = "TR_UDP"; break;     // сообщ. принято/отправлено с транспорт. уровня(udp)
      case VERIFY : use_flag_str = "VF"; break;             // верификация сообщения на уровне verification
      case PROT : use_flag_str = "PR"; break;               // обработка сообщения на уровне протокола(формирование/ парсинг команды)
      case MODEM_BEFORE : use_flag_str = "MD_BF"; break;    // сообщение отправлено на модуляцию/ демодуляцию
      case MODEM_AFTER : use_flag_str = "MD_AR"; break;     // сообщение после модуляции/демодуляции, отправляется на трансп. уровень для отправки клиенту
      default: use_flag_str = "UNKN"; break;
    }

    if (isOpen()){
        QTextStream logStream(this);

        if(m_format_time == " "){ // формат отображения времени не задан
           logStream << QString::number(num)<< " "
                     << QDateTime::currentDateTime().currentMSecsSinceEpoch()<<" "
                     << use_flag_str<<" "
                     << data;
           }
        else {
            logStream << QString::number(num)<< " "
                      << QDateTime::currentDateTime().toString(m_format_time)<<" "
                      << use_flag_str<<" "
                      << data;
            }
        logStream <<"\n";
        res = true;
     }

    return res;
}
//-----------------------------------------------------------------------------------------------------
void  LogMaker:: slotLogWrite(int num, LogUseEnum use_flag, QString &data)
{
    QString use_flag_str{"UNKN"};

    switch (use_flag){
      case TRANSP_TCP : use_flag_str = "TR_TCP"; break;     // сообщ. принято/отправлено с транспорт. уровня(tcp)
      case TRANSP_UDP : use_flag_str = "TR_UDP"; break;     // сообщ. принято/отправлено с транспорт. уровня(udp)
      case VERIFY : use_flag_str = "VF"; break;             // верификация сообщения на уровне verification
      case PROT : use_flag_str = "PR"; break;               // обработка сообщения на уровне протокола(формирование/ парсинг команды)
      case MODEM_BEFORE : use_flag_str = "MD_BF"; break;    // сообщение отправлено на модуляцию/ демодуляцию
      case MODEM_AFTER : use_flag_str = "MD_AR"; break;     // сообщение после модуляции/демодуляции, отправляется на трансп. уровень для отправки клиенту
      default: use_flag_str = "UNKN"; break;
    }

    if (isOpen()){
        QTextStream logStream(this);

        if(m_format_time == " "){ // формат отображения времени не задан
           logStream << QString::number(num)<< " "
                     << QDateTime::currentDateTime().currentMSecsSinceEpoch()
                     << use_flag_str
                     << data;
           }
        else {
            logStream << QString::number(num)<< " "
                      << QDateTime::currentDateTime().toString(m_format_time)
                      << use_flag_str
                      << data;
            }
        logStream <<"\n";

     }
}
//-------------------------------------------------------------------
bool LogMaker::removeOld(const QString fl_path)
{
    bool res{true};
   /* QString fl_path;
    QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
    QString LOG_PATH  = "/LOG/";               // дир. с логами.
    QString log_dir = APP_ABS_PATH + LOG_PATH;
    QString file_name = fl_name; //"log_mod_demod.txt";
    fl_path = APP_ABS_PATH + LOG_PATH + file_name;
    */
    QFile fl_log(fl_path);
    if(fl_log.exists()){
        res = fl_log.remove();
    }
    return res;
}
//-----------------------------------------------------------------------
