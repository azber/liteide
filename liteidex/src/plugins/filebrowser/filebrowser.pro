TARGET = filebrowser
TEMPLATE = lib

include(../../liteideplugin.pri)
include(../../api/golangdocapi/golangdocapi.pri)

DEFINES += FILEBROWSER_LIBRARY

SOURCES += filebrowserplugin.cpp \
    filebrowser.cpp \
    filebrowseroptionfactory.cpp \
    filebrowseroption.cpp \
    createfiledialog.cpp \
    createdirdialog.cpp

HEADERS += filebrowserplugin.h\
        filebrowser_global.h \
    filebrowser.h \
    filebrowseroptionfactory.h \
    filebrowseroption.h \
    createfiledialog.h \
    createdirdialog.h

RESOURCES += \
    filebrowser.qrc

FORMS += \
    filebrowseroption.ui \
    createfiledialog.ui \
    createdirdialog.ui
