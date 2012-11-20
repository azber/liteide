#-------------------------------------------------
#
# Project created by QtCreator 2011-03-18T16:21:43
#
#-------------------------------------------------
include (../../liteidex.pri)
include (../rpath.pri)
include (../api/liteapi/liteapi.pri)
include (../utils/fileutil/fileutil.pri)
include (../utils/mimetype/mimetype.pri)
include (../utils/textoutput/textoutput.pri)
include (../utils/extension/extension.pri)
include (../utils/filesystem/filesystem.pri)
include (../utils/symboltreeview/symboltreeview.pri)
include (../3rdparty/elidedlabel/elidedlabel.pri)

isEmpty(PRECOMPILED_HEADER):PRECOMPILED_HEADER = $$IDE_SOURCE_TREE/src/pch/liteide_gui_pch.h

QT += core gui

macx {
    QT += xml
}

DEFINES += LITEIDE_LIBRARY

TARGET = liteapp
DESTDIR = $$IDE_APP_PATH
TEMPLATE = app

contains(DEFINES, LITEIDE_LIBRARY) {
    TEMPLATE = lib
}

LIBS += -L$$IDE_LIBRARY_PATH

INCLUDEPATH += $$IDE_SOURCE_TREE/src/api
INCLUDEPATH += $$IDE_SOURCE_TREE/src/utils
INCLUDEPATH += $$IDE_SOURCE_TREE/src/3rdparty

SOURCES += main.cpp\
        mainwindow.cpp \
    liteapp.cpp \
    filemanager.cpp \
    editormanager.cpp \
    projectmanager.cpp \
    pluginmanager.cpp \
    dockmanager.cpp \
    outputmanager.cpp \
    actionmanager.cpp \
    aboutdialog.cpp \
    pluginsdialog.cpp \
    mimetypemanager.cpp \
    litetabwidget.cpp \
    optionmanager.cpp \
    newfiledialog.cpp \
    optionsbrowser.cpp \
    liteappoption.cpp \
    liteappoptionfactory.cpp \
    toolmainwindow.cpp \
    tooldockwidget.cpp \
    rotationtoolbutton.cpp \
    toolwindowmanager.cpp \
    folderproject.cpp \
    folderprojectfactory.cpp \
    goproxy.cpp

HEADERS  += mainwindow.h \
    liteapp.h \
    filemanager.h \
    editormanager.h \
    projectmanager.h \
    pluginmanager.h \
    dockmanager.h \
    outputmanager.h \
    actionmanager.h \
    aboutdialog.h \
    pluginsdialog.h \
    mimetypemanager.h \
    litetabwidget.h \
    optionmanager.h \
    newfiledialog.h \
    optionsbrowser.h \
    liteappoption.h \
    liteappoptionfactory.h \
    toolmainwindow.h \
    tooldockwidget.h \
    rotationtoolbutton.h \
    toolwindowmanager.h \
    folderproject.h \
    folderprojectfactory.h \
    goproxy.h

FORMS += \
    aboutdialog.ui \
    pluginsdialog.ui \
    newfiledialog.ui \
    optionswidget.ui \
    liteappoption.ui

RESOURCES += \
    liteapp.qrc

contains(DEFINES, LITEIDE_LIBRARY) {
    win32 {
        target.path = /bin
        INSTALLS += target
        RC_FILE += liteapp.rc
    } else:macx {
        ICON = images/liteide.icns
        QMAKE_INFO_PLIST = Info.plist
    } else {
        target.path  = /bin
        INSTALLS    += target
    }
}
