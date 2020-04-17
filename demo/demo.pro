QT = core

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += \
    main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -lqt-json
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -lqt-json
else:unix: LIBS += -L$$OUT_PWD/../lib/ -lqt-json

INCLUDEPATH += $$PWD/../src
DEPENDPATH += $$PWD/../src

DEFINES += "QTJSON_EXPORT=Q_DECL_IMPORT"