#ifndef CLOGGER_H
#define CLOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#define	SHOWBASE    	1             //  показывать префикс обозначения основания (0x - hex, 0b - bin o - восьмеричное )
#define	FORCESIGN	    2             //  знак перед числом, даже если это плюс
#define	FORCEPOINT    	4             //  десятичная точка, даже если дробная часть отсутствует
#define	UPCASEBASE	          0x08  // префикс базы в верхнем регистре
#define	UPCASEDIGITS	      0x10  // цифры hex- числа в верхнем регистре
#define	LEFT                       0x20  // выравнивание по левому краю поля вывода
#define	RIGHT       	          0x40  // выравнивание по правому краю
#define	CENTER 	              0x80  // выравнивание по центру


typedef void* CLogger;

// Конструктор и деструктор объекта-логгера.
CLogger  newLogger(char *logfileName, int killold, int do_log);
void   deleteLogger(CLogger clog);

// Публичные метода логгера. Принимают в качестве параметров указатель на объект-логгер и далее входные параметры для самого метода.
void log_info(CLogger clog, char* msg);
void log_warning(CLogger clog, char* msg);
void strToFile(CLogger clog, int head, char* msg, int newLine);
void intToFile(CLogger clog, int head, char*title, int number, int width, int base,char fillChar,int newLine);
void floatToFile(CLogger clog,int head, char*title, double value, int width, int precision, char format, int newLine);
void setFlags(CLogger clog,unsigned int flags);

#ifdef __cplusplus
}
#endif

#endif // CLOGGER_H
