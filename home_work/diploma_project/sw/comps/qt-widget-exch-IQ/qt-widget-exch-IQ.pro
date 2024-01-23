TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release
CONFIG += resources_big
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets
QT += network
QT += xml

INCLUDEPATH += inc
INCLUDEPATH += $$PWD/../../comps/fhss-protocol/inc

SOURCES += \
    src/widget-exch-IQ.cpp
    
HEADERS += \
    inc/widget-exch-IQ.h

FORMS += \
       src/widget-exch-IQ.ui
