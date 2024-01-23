TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets

INCLUDEPATH += inc
INCLUDEPATH += src
INCLUDEPATH += $$PWD/../../comps/eml/inc
INCLUDEPATH += $$PWD/../../comps/qt-dce-panel/inc
INCLUDEPATH += $$PWD/../../comps/fhss-logger/inc

SOURCES += \
    src/chanmodel.cpp \
    src/colorgrid.cpp \
    src/fadings.cpp

HEADERS += \
    inc/chanmodel.h \
    inc/colorgrid.h \
    src/fadings.h \
    src/filters.h

