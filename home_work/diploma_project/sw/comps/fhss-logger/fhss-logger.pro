TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release

INCLUDEPATH += inc

HEADERS += \
    inc/clogger.h \
    inc/logger.h

SOURCES += \
    src/clogger.cpp \
    src/logger.cpp

android { 
	DEFINES += DISABLE_LOG
}