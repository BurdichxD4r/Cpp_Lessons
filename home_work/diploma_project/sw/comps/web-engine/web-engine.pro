
# This project demonstrates how to use QtWebAppLib by including the
# sources into this project.


TEMPLATE = lib
CONFIG += staticlib
QT = core network
CONFIG += console
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

INCLUDEPATH += inc

INCLUDEPATH += $$PWD/../../comps/QtWebApp/httpserver
INCLUDEPATH += $$PWD/../../comps/QtWebApp/logging
INCLUDEPATH += $$PWD/../../comps/QtWebApp/templateengine

SOURCES += \
     src/plotcontroller.cpp \
     src/admincontroller.cpp \
     src/global.cpp \
     src/secureservice.cpp \
     src/helpcontroller.cpp \
     src/logincontroller.cpp \
     src/settingcontroller.cpp \     
     src/requesthandler.cpp \
     src/summarycontroller.cpp \
     src/webengine.cpp
    
HEADERS += \
    inc/plotcontroller.h \
    inc/admincontroller.h \
    inc/global.h \
    inc/secureservice.h \
    inc/helpcontroller.h \
    inc/logincontroller.h \
    inc/settingcontroller.h \    
    inc/requesthandler.h \
    inc/summarycontroller.h \
    inc/webengine.h

OTHER_FILES += etc/* logs/* ../readme.txt






