TEMPLATE = lib

QT = core testlib
CONFIG += staticlib c++17 exceptions

HEADERS += \
    serializationtest.h \
    testlib.h \
    testserializable.h

SOURCES += \
    serializationtest.cpp \
    testlib.cpp \
    testserializable.cpp

IS_TESTLIB = true
include(../tests.pri)
