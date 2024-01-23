TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release

INCLUDEPATH += inc

INCLUDEPATH += $$PWD/../../comps/qt-customplot/inc

HEADERS += \
    inc/dceplot.h \

SOURCES += \
    src/dceplot.cpp \
