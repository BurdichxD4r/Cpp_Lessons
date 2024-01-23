#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QTextStream>

enum VAL_TYPE {    // тип логируемого значения
    TXT_TYPE     = 0,
    INT_TYPE     = 1,
    FLOAT_TYPE = 2,
};

typedef struct {
  char  vtype;     // тип логируемого значения
  char* txt;        // - символьная строка
  int number;     // - целое число
  int width;        // - ширина поля вывода
  int base;         // - основание числа(2,8,10,16)
  char fillChar;   // - символ, которым заполняется поле вывода до width
  double value;  // - вещественное число
  int precision;   // - число знаков после десятичной точки
  char format;   //  - формат вывода веществ.числа('e','f','g','E','F')
} log_T;

class QFile;

class Logger
{
public:
    Logger(QString fileName, int killOldFile, int do_log);
    ~Logger();

    void log_info(QString msg);
    void log_warning(QString msg);
    void writingToFile(int head, log_T val, int newLine);
    void setStreamFlags(quint32 flags);
    void clearFlags();
    bool doingLog();

private:
    QString logName;
    QString tmFormat = "dd.MM.yyyy hh:mm:ss.zzz";
    QFile* logFile = nullptr;
    QTextStream* tsTextStream;
    bool log_mode = true;
    bool reOpenFile = false;

    //QTextStream::NumberFlags numflags ;
    bool showBase = false;
    bool forceSign = false;
    bool forcePoint = false;
    bool upperCaseBase = false;
    bool upperCaseDigits = false;
    bool leftAlign = false;
    bool rightAlign = false;
    bool centerAlign = false;

    void printTimeStamp(QTextStream *ts);
    void openLogFile();
};

#endif // LOGGER_H
