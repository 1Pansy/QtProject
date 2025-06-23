 QT       += core gui
QT += sql

QT += charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DEFINES += QT_CHARTS_USE_NAMESPACE
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    change_password_dialog.cpp \
    charts.cpp \
    dlg_user.cpp \
    dlgdata.cpp \
    log_viewer.cpp \
    main.cpp \
    mainwindow.cpp \
    page_login.cpp \
    usersql.cpp

HEADERS += \
    change_password_dialog.h \
    charts.h \
    dlg_user.h \
    dlgdata.h \
    log_viewer.h \
    mainwindow.h \
    page_login.h \
    usersql.h

FORMS += \
    charts.ui \
    dlg_user.ui \
    dlgdata.ui \
    mainwindow.ui \
    page_login.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
