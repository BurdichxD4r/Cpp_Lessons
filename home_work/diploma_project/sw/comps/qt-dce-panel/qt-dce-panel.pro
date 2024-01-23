TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets

INCLUDEPATH += inc
INCLUDEPATH += $$PWD/../../comps/eml/inc
INCLUDEPATH += $$PWD/../../comps/qt-dce-plot/inc
INCLUDEPATH += $$PWD/../../comps/fhss-logger/inc
INCLUDEPATH += $$PWD/../../comps/prsgen/inc

SOURCES += \
    src/adapt_speed.cpp \
    src/fhssmodem.cpp \
    src/modemchart.cpp \
    src/fhssslot.cpp \
    src/modempanel.cpp \
    src/tadaptfreq.cpp \
    src/tbestfreq.cpp \
    src/tconnect.cpp \
    src/tconsole.cpp \
    src/tsettings.cpp \
    src/tstream.cpp

HEADERS += \
    inc/modempanel.h \
    src/adapt_speed.h \
    src/connection.h \
    src/modemchart.h \
    src/fhssmodem.h \
    src/fhssslot.h \
    src/slot_params.h \
    src/tadaptfreq.h \
    src/tbestfreq.h \
    src/tconnect.h \
    src/tconsole.h \
    src/tsettings.h \
    src/tstream.h

