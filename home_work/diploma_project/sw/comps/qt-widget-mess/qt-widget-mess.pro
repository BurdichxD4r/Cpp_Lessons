TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets

INCLUDEPATH += inc

SOURCES += \
    src/widgetmess.cpp \ 
    src/tablemess.cpp \
    src/textmess.cpp \
    src/textcmd.cpp
    

HEADERS += \
    inc/widgetmess.h \
    inc/tablemess.h \
    inc/textmess.h \
    inc/textcmd.h 

