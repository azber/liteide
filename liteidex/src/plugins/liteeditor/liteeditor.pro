TARGET = liteeditor
TEMPLATE = lib

QT += xml

include (../../liteideplugin.pri)
include (../../api/liteeditorapi/liteeditorapi.pri)
include (../../utils/extension/extension.pri)
include (../../utils/mimetype/mimetype.pri)
include (../../utils/wordapi/wordapi.pri)
include (../../utils/colorstyle/colorstyle.pri)
include (../../3rdparty/qtc_texteditor/qtc_texteditor.pri)
include (../../3rdparty/treemodelcompleter/treemodelcompleter.pri)

DEFINES += LITEEDITOR_LIBRARY

SOURCES += liteeditorplugin.cpp \
    liteeditor.cpp \
    liteeditorfile.cpp \
    liteeditorfilefactory.cpp \
    golanghighlighter.cpp \
    liteeditoroption.cpp \
    liteeditoroptionfactory.cpp \
    liteeditorwidgetbase.cpp \
    liteeditorwidget.cpp \
    litecompleter.cpp \
    litewordcompleter.cpp \
    wordapimanager.cpp

HEADERS += liteeditorplugin.h\
        liteeditor_global.h \
    liteeditor.h \
    liteeditorfile.h \
    liteeditorfilefactory.h \
    golanghighlighter.h \
    liteeditoroption.h \
    liteeditoroptionfactory.h \
    liteeditorwidgetbase.h \
    liteeditorwidget.h \
    litecompleter.h \
    litewordcompleter.h \
    wordapimanager.h

FORMS += \
    liteeditoroption.ui

RESOURCES += \
    liteeditor.qrc
