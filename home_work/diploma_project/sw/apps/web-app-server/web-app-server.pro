# This project demonstrates how to use QtWebAppLib by including the
# sources into this project.

TARGET = web-app-server
TEMPLATE = app
QT = core network
CONFIG += console
CONFIG += c++11
CONFIG -= debug_and_release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \           
           src/startup.h


SOURCES += src/main.cpp \          
           src/startup.cpp


INCLUDEPATH += $$PWD/../../comps/QtWebApp/httpserver
INCLUDEPATH += $$PWD/../../comps/QtWebApp/logging
INCLUDEPATH += $$PWD/../../comps/QtWebApp/templateengine
INCLUDEPATH += $$PWD/../../comps/QtWebApp/qtservice
INCLUDEPATH += $$PWD/../../comps/web-engine/inc


PRE_TARGETDEPS += \
        $$OUT_PWD/../../comps/web-engine/libweb-engine.a
        PRE_TARGETDEPS += \
                        $$OUT_PWD/../../comps/QtWebApp/libQtWebApp.a


LIBS += -L$$OUT_PWD/../../comps/web-engine
LIBS += -L$$OUT_PWD/../../comps/QtWebApp


LIBS += -lweb-engine
LIBS += -lQtWebApp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    etc/docroot/admin_templ-ru.dhtml \
    etc/docroot/authen_templ-ru.dhtml \
    etc/docroot/hc_templ-ru.dhtml \
    etc/docroot/jq_templ-ru.dhtml \
    etc/docroot/plot_templ-ru.dhtml \
    etc/docroot/sett_templ-ru.dhtml \
    etc/docroot/style.css \
    etc/docroot/summary_templ-ru.dhtml \
    etc/secureroot/web-app-server.txt \
    etc/testroot/test_modem_exch.txt \
    etc/testroot/test_sett.txt \
    etc/testroot/test_summary.txt \
    etc/testroot/test_summary_sett.txt \
    etc/web-app-server.ini
