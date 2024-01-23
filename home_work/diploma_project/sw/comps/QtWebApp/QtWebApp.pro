# Build this project to generate a shared library (*.dll or *.so).
TEMPLATE = lib
CONFIG += staticlib
QT = core network
CONFIG += console
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT


INCLUDEPATH += $$PWD/../../comps/QtWebApp/httpserver
INCLUDEPATH += $$PWD/../../comps/QtWebApp/logging
INCLUDEPATH += $$PWD/../../comps/QtWebApp/templateengine
INCLUDEPATH += $$PWD/../../comps/QtWebApp/qtservice


include(logging/logging.pri)
include(httpserver/httpserver.pri)
include(templateengine/templateengine.pri)
include(qtservice/qtservice.pri)


