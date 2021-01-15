QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    multiplay.cpp

HEADERS += \
    mainwindow.h \
    multiplay.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    client_ko_KR.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../connect6_protocol/release/ -lconnect6_protocol
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../connect6_protocol/debug/ -lconnect6_protocol
else:unix: LIBS += -L$$PWD/../connect6_protocol/ -lconnect6_protocol

INCLUDEPATH += $$PWD/../connect6_protocol
DEPENDPATH += $$PWD/../connect6_protocol
