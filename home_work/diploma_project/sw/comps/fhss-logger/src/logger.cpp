#include "logger.h"
#include "clogger.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QCoreApplication>
#include <QDir>

//#define DISABLE_LOG

Logger::Logger(QString fileName, int killOldFile, int do_log)
{
#if defined (DISABLE_LOG)
	do_log = 0;
#endif
    if (do_log) {
        log_mode = true;
        if (killOldFile) reOpenFile = true;
        logName = fileName;
    }else{
        log_mode = false;
    }
}

Logger::~Logger()
{
    if (logFile != nullptr){
        if (logFile->isOpen()){
            delete tsTextStream;
            logFile->close();
        }
        delete logFile;
    }
}

void Logger::openLogFile()
{
    QString fname;
    QString dirName = QCoreApplication::applicationDirPath()+"/FHSS";
    QDir fDir = QDir(dirName);
    if (!fDir.exists()) fDir.mkdir(dirName);
    fname = dirName +"/"+logName;

    if (reOpenFile){
        QFile fMessFile(fname);
        if(fMessFile.open(QFile::WriteOnly | QFile::Text)){
             fMessFile.close();
        }
    }
    logFile = new QFile(fname);
    if(!logFile->open(QFile::Append | QFile::Text)){
        return;
    }
    tsTextStream = new QTextStream(logFile);
}

bool Logger::doingLog()
{
    return log_mode;
}

void Logger::printTimeStamp(QTextStream *ts)
{
    QString sCurrDateTime = QDateTime::currentDateTime().toString(tmFormat) ;
    (*ts) << QString("%1 ").arg(sCurrDateTime);
}

void Logger::writingToFile(int head, log_T val, int newLine)
{
    QString str;
    if (logFile == nullptr) openLogFile();
    if (logFile->isOpen())
    {
        QTextStream::NumberFlags flags ;
        if (showBase) flags = flags | QTextStream::ShowBase;
        if (forcePoint) flags = flags | QTextStream::ForcePoint;
        if (forceSign)  flags = flags |  QTextStream::ForceSign;
        if (upperCaseBase) flags = flags | QTextStream::UppercaseBase;
        if (upperCaseDigits) flags = flags | QTextStream::UppercaseDigits;

        if (head) printTimeStamp(tsTextStream);
        *tsTextStream << val.txt;

        if (leftAlign) tsTextStream->setFieldAlignment(QTextStream::AlignLeft);
        else if (rightAlign) tsTextStream->setFieldAlignment(QTextStream::AlignRight);
        else if (centerAlign) tsTextStream->setFieldAlignment(QTextStream::AlignCenter);

        tsTextStream->setNumberFlags(flags);
        tsTextStream->setFieldWidth(val.width);

        switch(val.vtype){
        case INT_TYPE:

            tsTextStream->setIntegerBase(val.base);
            tsTextStream->setPadChar(val.fillChar);
            *tsTextStream << val.number ;
            break;

        case FLOAT_TYPE:

            tsTextStream->setRealNumberPrecision(val.precision);
            if(val.format == 'e') tsTextStream->setRealNumberNotation(tsTextStream->ScientificNotation);
            else if(val.format == 'g') tsTextStream->setRealNumberNotation(tsTextStream->SmartNotation);
            else tsTextStream->setRealNumberNotation(tsTextStream->FixedNotation);
            *tsTextStream << val.value ;
            break;

        default:

            break;

        }

        if (newLine) *tsTextStream << endl;

        tsTextStream->flush();
        tsTextStream->reset();
        logFile->flush();
        clearFlags();
    }
}

void Logger::setStreamFlags(quint32 flags)
{
    if (flags & SHOWBASE) showBase = true;
    if (flags & FORCESIGN) forceSign = true;
    if (flags & FORCEPOINT) forcePoint = true;
    if (flags & UPCASEBASE) upperCaseBase = true;
    if (flags & UPCASEDIGITS) upperCaseDigits = true;
    if (flags & LEFT) leftAlign = true;
    if (flags & RIGHT) rightAlign = true;
    if (flags & CENTER) centerAlign = true;
    //numflags = QTextStream::NumberFlags(flags & 0x1f);
}

void Logger::clearFlags()
{
    showBase = false;
    forceSign = false;
    forcePoint = false;
    upperCaseBase = false;
    upperCaseDigits = false;
    leftAlign = false;
    rightAlign = false;
    centerAlign = false;
    //numflags = QTextStream::NumberFlags();
}

void Logger::log_info(QString msg)
{
    Q_UNUSED(msg)
    //C++ реализация как обычно для класса
    qDebug() << msg;
}

void Logger::log_warning(QString msg)
{
    Q_UNUSED(msg)
    qDebug() << msg;
    //C++ реализация как обычно для класса
}
