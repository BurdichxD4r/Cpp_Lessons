TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release
CONFIG += resources_big
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT += widgets
QT += network
QT += xml

INCLUDEPATH += inc
INCLUDEPATH += $$PWD/../../comps/qt-simple-plot/inc
INCLUDEPATH += $$PWD/../../comps/eml/inc
INCLUDEPATH += $$PWD/../../comps/qt-dce-panel/inc
INCLUDEPATH += $$PWD/../../comps/qt-dce-plot/inc
INCLUDEPATH += $$PWD/../../comps/fhss-logger/inc
INCLUDEPATH += $$PWD/../../comps/prsgen/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-exch-IQ/inc
INCLUDEPATH += $$PWD/../../comps/qt-widget-mess/inc
INCLUDEPATH += $$PWD/../../comps/fhss-protocol/inc


SOURCES += \
    src/coreDCE.cpp \ 
    src/widget_dce.cpp \ 
    src/dte_label.cpp
    
HEADERS += \
    inc/coreDCE.h \
    inc/dte_label.h \
    inc/widget_dce.h 

FORMS += \
       src/widget_dce.ui

RESOURCES += \
    pict.qrc
