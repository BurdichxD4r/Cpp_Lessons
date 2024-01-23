TEMPLATE = lib
CONFIG += staticlib
CONFIG -= debug_and_release

#CONFIG += console c++11
#CONFIG -= app_bundle
#CONFIG -= qt

INCLUDEPATH += inc

HEADERS += \
    inc/psrgen.h \
    inc/t_bentfunction.h \
    inc/t_bitstring.h \
    inc/t_shiftregister.h \
    inc/t_matrix.h \
    inc/t_md5.h

SOURCES += \
    src/psrgen.cpp \
    src/t_bentfunction.cpp \
    src/t_bitstring.cpp \
    src/t_shiftregister.cpp \
    src/t_matrix.cpp \
    src/t_md5.cpp

