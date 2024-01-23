#include "logger.h"
#include "clogger.h"

CLogger  newLogger(char* logfileName, int killold, int do_log)
{
    Logger *log = new Logger(QString(logfileName),killold, do_log);
    return static_cast<CLogger>(log);
}

void   deleteLogger(CLogger clog)
{
    Logger *log = static_cast<Logger*>(clog);
    delete log;
}

void log_info(CLogger clog, char*msg)
{
    Logger *log = static_cast<Logger*>(clog);
    log->log_info(QString(msg));
}

void log_warning(CLogger clog, char*msg)
{
    Logger *log = static_cast<Logger*>(clog);
    log->log_warning(QString(msg));
}

void strToFile(CLogger clog, int head, char*msg, int newLine)
{
    Logger *log = static_cast<Logger*>(clog);
    if(log->doingLog()){
        log_T vlog;
        vlog.vtype = TXT_TYPE;
        vlog.txt = msg;
        vlog.width = 0;
        log->writingToFile(head, vlog, newLine);
    }
}

void intToFile(CLogger clog, int head, char*title, int number, int width, int base,char fillChar, int newLine)
{
    Logger *log = static_cast<Logger*>(clog);
    if(log->doingLog()){
        log_T vlog;
        vlog.vtype = INT_TYPE;
        vlog.txt = title;
        vlog.number = number;
        vlog.width = width;
        vlog.base = base;
        vlog.fillChar = fillChar;
        log->writingToFile(head, vlog, newLine);
    }
}

void floatToFile(CLogger clog,int head, char*title, double value, int width, int precision, char format, int newLine)
{
    Logger *log = static_cast<Logger*>(clog);
    if(log->doingLog()){
        log_T vlog;
        vlog.vtype = FLOAT_TYPE;
        vlog.txt = title;
        vlog.value = value;
        vlog.width = width;
        vlog.precision = precision;
        vlog.format = format;
        log->writingToFile(head, vlog, newLine);
    }
}

void setFlags(CLogger clog, unsigned int flags)
{
    Logger *log = static_cast<Logger*>(clog);
    if(log->doingLog()){
        log->setStreamFlags(flags);
    }
}
