/**************************************************************************
** This file is part of LiteIDE
**
** Copyright (c) 2011 LiteIDE Team. All rights reserved.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** In addition, as a special exception,  that plugins developed for LiteIDE,
** are allowed to remain closed sourced and can be distributed under any license .
** These rights are included in the file LGPL_EXCEPTION.txt in this package.
**
**************************************************************************/
// Module: golangdoc.cpp
// Creator: visualfc <visualfc@gmail.com>
// date: 2011-7-7
// $Id: golangdoc.cpp,v 1.0 2011-7-7 visualfc Exp $

#include "golangdoc.h"
#include "liteapi/litefindobj.h"
#include "litebuildapi/litebuildapi.h"
#include "processex/processex.h"
#include "fileutil/fileutil.h"
#include "htmlutil/htmlutil.h"
#include "golangapi/golangapi.h"
#include "documentbrowser/documentbrowser.h"

#include <QListView>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QToolBar>
#include <QStatusBar>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QDir>
#include <QTextBrowser>
#include <QUrl>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QGroupBox>
#include <QToolButton>
#include <QTextCodec>
#include <QDesktopServices>
#include <QDomDocument>
#include <QDebug>
//lite_memory_check_begin
#if defined(WIN32) && defined(_MSC_VER) &&  defined(_DEBUG)
     #define _CRTDBG_MAP_ALLOC
     #include <stdlib.h>
     #include <crtdbg.h>
     #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
     #define new DEBUG_NEW
#endif
//lite_memory_check_end


GolangDoc::GolangDoc(LiteApi::IApplication *app, QObject *parent) :
    LiteApi::IGolangDoc(parent),
    m_liteApp(app)
{
    m_findProcess = new ProcessEx(this);
    m_godocProcess = new ProcessEx(this);

    m_widget = new QWidget;
    m_findResultModel = new QStringListModel(this);
    m_findFilterModel = new QSortFilterProxyModel(this);
    m_findFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_findFilterModel->setSourceModel(m_findResultModel);

    m_findResultListView = new QListView;
    m_findResultListView->setEditTriggers(0);
    m_findResultListView->setModel(m_findFilterModel);

    m_findEdit = new Utils::FilterLineEdit;
    m_golangApi = new GolangApi(this);
    //m_findEdit->setEditable(true);
    //m_findEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    //m_findEdit->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

    //m_findAct = new QAction(tr("Find"),this);
    //m_listPkgAct = new QAction(tr("List \"src/pkg\""),this);
    //m_listCmdAct = new QAction(tr("List \"src/cmd\""),this);

    //m_findMenu = new QMenu(tr("Find"));
    //m_findMenu->addAction(m_findAct);
    //m_findMenu->addSeparator();
    //m_findMenu->addAction(m_listPkgAct);
    //m_findMenu->addAction(m_listCmdAct);

    //QToolButton *findBtn = new QToolButton;
    //findBtn->setPopupMode(QToolButton::MenuButtonPopup);
    //findBtn->setDefaultAction(m_findAct);
    //findBtn->setMenu(m_findMenu);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(1);
    mainLayout->setSpacing(1);

    QHBoxLayout *findLayout = new QHBoxLayout;
    findLayout->setMargin(2);
    findLayout->addWidget(new QLabel("Find"));
    findLayout->addWidget(m_findEdit);
    //findLayout->addWidget(findBtn);

    mainLayout->addLayout(findLayout);
    mainLayout->addWidget(m_findResultListView);
    m_widget->setLayout(mainLayout);

    m_liteApp->dockManager()->addDock(m_widget,tr("Golang Document Find"),Qt::LeftDockWidgetArea);

    m_docBrowser = new DocumentBrowser(m_liteApp,this);
    m_docBrowser->setName(tr("Golang Document Browser"));
    QStringList paths;
    paths << m_liteApp->resourcePath()+"/golangdoc";
    m_docBrowser->setSearchPaths(paths);

    m_godocFindComboBox = new QComboBox;
    m_godocFindComboBox->setEditable(true);
    m_docBrowser->toolBar()->addSeparator();
    m_docBrowser->toolBar()->addWidget(m_godocFindComboBox);

    m_browserAct = m_liteApp->editorManager()->registerBrowser(m_docBrowser);
    m_liteApp->actionManager()->insertViewMenu(LiteApi::ViewMenuPagePos,m_browserAct);

    connect(m_docBrowser,SIGNAL(requestUrl(QUrl)),this,SLOT(openUrl(QUrl)));
    connect(m_docBrowser,SIGNAL(highlighted(QUrl)),this,SLOT(highlighted(QUrl)));    
    connect(m_godocFindComboBox,SIGNAL(activated(QString)),this,SLOT(godocFindPackage(QString)));
    connect(m_findEdit,SIGNAL(filterChanged(QString)),m_findFilterModel,SLOT(setFilterFixedString(QString)));
    //connect(m_findEdit,SIGNAL(activated(QString)),this,SLOT(findPackage(QString)));
    connect(m_godocProcess,SIGNAL(extOutput(QByteArray,bool)),this,SLOT(godocOutput(QByteArray,bool)));
    connect(m_godocProcess,SIGNAL(extFinish(bool,int,QString)),this,SLOT(godocFinish(bool,int,QString)));
    connect(m_findProcess,SIGNAL(extOutput(QByteArray,bool)),this,SLOT(findOutput(QByteArray,bool)));
    connect(m_findProcess,SIGNAL(extFinish(bool,int,QString)),this,SLOT(findFinish(bool,int,QString)));
    connect(m_findResultListView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(doubleClickListView(QModelIndex)));
    //connect(m_findAct,SIGNAL(triggered()),this,SLOT(findPackage()));
    //connect(m_listPkgAct,SIGNAL(triggered()),this,SLOT(listPkg()));
    //connect(m_listCmdAct,SIGNAL(triggered()),this,SLOT(listCmd()));

    m_envManager = LiteApi::findExtensionObject<LiteApi::IEnvManager*>(m_liteApp,"LiteApi.IEnvManager");
    if (m_envManager) {
        connect(m_envManager,SIGNAL(currentEnvChanged(LiteApi::IEnv*)),this,SLOT(currentEnvChanged(LiteApi::IEnv*)));
        currentEnvChanged(m_envManager->currentEnv());
    }

    m_liteApp->extension()->addObject("LiteApi.IGolangDoc",this);
    m_liteApp->extension()->addObject("LiteApi.IGolangApi",m_golangApi);

    QString path = m_liteApp->resourcePath()+"/golangdoc/godoc.html";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        m_templateData = file.readAll();
        file.close();
    }
    QString about = m_liteApp->resourcePath()+"/golangdoc/about.html";
    QFileInfo info(about);
    if(info.exists()) {
        m_templateData.replace("{about}",info.filePath());
    }

    QUrl url;
    url.setScheme("file");
    url.setPath(info.filePath());
    openUrl(url);
}

GolangDoc::~GolangDoc()
{
    m_liteApp->settings()->setValue("golangdoc/goroot",m_goroot);
    if (m_docBrowser) {
        delete m_docBrowser;
    }
    //delete m_findMenu;
    delete m_widget;
}

void GolangDoc::loadApi()
{
    QString goroot = LiteApi::getGoroot(m_liteApp);
    QFileInfo info(goroot,"api/go1.txt");
    if (!info.exists()) {
        QString path = m_liteApp->resourcePath()+"/golangdoc";
        info.setFile(path,"go1.txt");
        if (!info.exists())
            return;
    }
    if (m_golangApi->load(info.filePath())) {
        m_findResultModel->setStringList(m_golangApi->all(LiteApi::AllGolangApi));//&~LiteApi::ConstApi));
    }
}

void GolangDoc::currentEnvChanged(LiteApi::IEnv*)
{
    QProcessEnvironment env = m_envManager->currentEnvironment();
    QString goroot = env.value("GOROOT");
    QString gobin = env.value("GOBIN");
    if (!goroot.isEmpty() && gobin.isEmpty()) {
        gobin = goroot+"/bin";
    }
    QString godoc = FileUtil::findExecute(gobin+"/godoc");
    if (godoc.isEmpty()) {
        godoc = FileUtil::lookPath("godoc",env,true);
    }
    QString find = FileUtil::findExecute(m_liteApp->applicationPath()+"/godocview");
    if (find.isEmpty()) {
        find = FileUtil::lookPath("godocview",env,true);
    }

    m_goroot = goroot;
    m_godocCmd = godoc;
    m_findCmd = find;

    m_findProcess->setEnvironment(env.toStringList());
    m_godocProcess->setEnvironment(env.toStringList());
    this->loadApi();
}

void GolangDoc::activeBrowser()
{
    m_liteApp->editorManager()->activeBrowser(m_docBrowser);
}

void GolangDoc::listPkg()
{
    if (m_findCmd.isEmpty()) {
        return;
    }
    QStringList args;
    args << "-mode=lite" << "-list=pkg";
    m_findData.clear();
    m_findProcess->start(m_findCmd,args);
}

void GolangDoc::listCmd()
{
    if (m_findCmd.isEmpty()) {
        return;
    }
    QStringList args;
    args << "-mode=lite" << "-list=cmd";
    m_findData.clear();
    m_findProcess->start(m_findCmd,args);
}

void GolangDoc::godocFindPackage(QString pkgname)
{
    if (pkgname.isEmpty()) {
        pkgname = m_godocFindComboBox->currentText();
    }
    if (pkgname.isEmpty()) {
        return;
    }
    QUrl url;
    url.setScheme("find");
    url.setPath(pkgname);
    openUrl(url);
}

void GolangDoc::findPackage(QString pkgname)
{
    if (pkgname.isEmpty()) {
        pkgname = m_findEdit->text();
    }
    if (pkgname.isEmpty()) {
        return;
    }
    if (m_findCmd.isEmpty()) {
        return;
    }
    QStringList args;
    args << "-mode=lite" << "-find" << pkgname;
    m_findData.clear();
    m_findProcess->start(m_findCmd,args);
}

void GolangDoc::findOutput(QByteArray data,bool bStderr)
{
    if (bStderr) {
        return;
    }
    m_findData.append(data);
}

void GolangDoc::findFinish(bool error,int code,QString /*msg*/)
{
    if (!error && code == 0) {
        QStringList array = QString(m_findData.trimmed()).split(',');
        if (array.size() >= 2 && array.at(0) == "$find") {
            array.removeFirst();
            QString best = array.at(0);
            if (best.isEmpty()) {
                array.removeFirst();
            } else {
                activeBrowser();
                QUrl url;
                url.setScheme("pdoc");
                url.setPath(best);
                openUrl(url);
            }
            if (array.isEmpty()) {
                m_findResultModel->setStringList(QStringList() << "<nofind>");
            } else {
                m_findResultModel->setStringList(array);
            }
        } else if (array.size() >= 1 && array.at(0) == "$list") {
            array.removeFirst();
            m_findResultModel->setStringList(array);
        }
    } else {
        m_findResultModel->setStringList(QStringList() << "<error>");
    }
}

void GolangDoc::godocOutput(QByteArray data,bool bStderr)
{
    if (bStderr) {
        return;
    }
    m_godocData.append(data);
}

void GolangDoc::godocFinish(bool error,int code,QString /*msg*/)
{
    if (!error && code == 0 && m_docBrowser != 0) {
        bool nav = true;
        QString header;
        if (m_openUrl.scheme() == "list") {
            nav = false;
            header = "Package List";
        } else if (m_openUrl.scheme() == "find") {
            nav = false;
            header = "Find Package "+m_openUrl.path();
        } else if (m_openUrl.scheme() == "pdoc") {
            nav = false;
            header = "Package "+m_openUrl.path();
        }
        updateHtmlDoc(m_openUrl,m_godocData,header,nav);
    }
}

void GolangDoc::updateTextDoc(const QUrl &url, const QByteArray &ba, const QString &header)
{
    m_lastUrl = url;
    QTextCodec *codec = QTextCodec::codecForUtfText(ba,QTextCodec::codecForName("utf-8"));
    QString html = Qt::escape(codec->toUnicode(ba));
    QString data = m_templateData;
    data.replace("{header}",header);
    data.replace("{nav}","");
    data.replace("{content}",QString("<pre>%1</pre>").arg(html));
    m_docBrowser->setUrlHtml(url,data);

}

void GolangDoc::updateHtmlDoc(const QUrl &url, const QByteArray &ba, const QString &header, bool toNav)
{
    m_lastUrl = url;
    if (m_lastUrl.scheme() == "pdoc") {
        m_targetList.clear();
        QString pkgname = m_lastUrl.path();
        QString goroot = LiteApi::getGoroot(m_liteApp);
        QFileInfo i1(QFileInfo(goroot,"src/cmd").filePath(),pkgname);
        if (i1.exists()) {
            m_targetList.append(i1.filePath());
        }
        QFileInfo i2(QFileInfo(goroot,"src/pkg").filePath(),pkgname);
        if (i2.exists()) {
            m_targetList.append(i2.filePath());
        }
        foreach(QString path,LiteApi::getGopathList(m_liteApp,false)) {
            QFileInfo info(QFileInfo(path,"src").filePath(),pkgname);
            if (info.exists()) {
                m_targetList.append(info.filePath());
            }
        }
    }

    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString genHeader;
    QString nav;

    QString content = HtmlUtil::docToNavdoc(codec->toUnicode(ba),genHeader,nav);
    QString data = m_templateData;

    if (genHeader.isEmpty()) {
        data.replace("{header}",header);
    } else {
        data.replace("{header}",genHeader);
    }
    if (toNav) {
        data.replace("{nav}",nav);
    } else {
        data.replace("{nav}","");
    }
    data.replace("{content}",content);
    m_docBrowser->setUrlHtml(url,data);
}

void GolangDoc::openUrlList(const QUrl &url)
{
    if (url.scheme() != "list") {
        return;
    }
    if (m_findCmd.isEmpty()) {
        return;
    }
    QStringList args;
    args << "-mode=html"<< QString("-list=%1").arg(url.path());
    m_godocData.clear();
    m_godocProcess->start(m_findCmd,args);
}

void GolangDoc::openUrlFind(const QUrl &url)
{
    if (url.scheme() != "find") {
        return;
    }
    if (m_findCmd.isEmpty()) {
        return;
    }
    QStringList args;
    args << "-mode=html" << "-find" << url.path();
    m_godocData.clear();
    m_godocProcess->start(m_findCmd,args);
    return;
}

void GolangDoc::openUrlPdoc(const QUrl &url)
{
    if (url.scheme() != "pdoc") {
        return;
    }

    if (m_godocCmd.isEmpty()) {
        return;
    }
    if (m_godocProcess->isRuning()) {
        m_godocProcess->waitForFinished(200);
    }
    m_godocData.clear();
    QStringList args;
    //check additional path
    bool additional = false;
    if (QDir(url.path()).exists()) {
        additional = true;
        if (url.path().length() >= 2 && url.path().right(2) == "..") {
            additional = false;
        }
    }
    if (additional) {
        m_godocProcess->setWorkingDirectory(url.path());
        args << "-html=true" << "-path=." << ".";
    } else {
        m_godocProcess->setWorkingDirectory(m_goroot);
        args << "-html=true" << url.path();
    }
    m_godocProcess->setEnvironment(LiteApi::getGoEnvironment(m_liteApp).toStringList());
    m_godocProcess->start(m_godocCmd,args);
}

void GolangDoc::openUrlFile(const QUrl &url)
{
    QFileInfo info(url.toLocalFile());
    if (!info.exists()) {
        info.setFile(url.path());
    }
    QString ext = info.suffix().toLower();
    if (ext == "html") {
        QFile file(info.filePath());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray ba = file.readAll();
            file.close();
            if (info.fileName().compare("docs.html",Qt::CaseInsensitive) == 0) {
                updateHtmlDoc(url,ba,info.fileName(),false);
            } else {
                updateHtmlDoc(url,ba,info.fileName(),true);
            }
        }
    } else if (ext == "go") {
        LiteApi::IEditor *editor = m_liteApp->editorManager()->findEditor(info.filePath(),true);
        if (!editor) {
            editor = m_liteApp->fileManager()->openEditor(info.filePath(),true);
            editor->setReadOnly(true);
        } else {
            m_liteApp->editorManager()->setCurrentEditor(editor);
        }
        if (editor) {
            QPlainTextEdit *ed = LiteApi::findExtensionObject<QPlainTextEdit*>(editor,"LiteApi.QPlainTextEdit");
            if (ed && url.hasQueryItem("s")) {
                QStringList pos = url.queryItemValue("s").split(":");
                if (pos.length() == 2) {
                    bool ok = false;
                    int begin = pos.at(0).toInt(&ok);
                    if (ok) {
                        QTextCursor cur = ed->textCursor();
                        cur.setPosition(begin);
                        ed->setTextCursor(cur);
                        ed->centerCursor();
                    }
                }
            }
        }
    } else if (ext == "pdf") {
        QDesktopServices::openUrl(info.filePath());
    } else {
        QFile file(info.filePath());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray ba = file.readAll();
            updateTextDoc(url,ba,info.fileName());
        }
    }
}

QUrl GolangDoc::parserUrl(const QUrl &_url)
{
    QUrl url = _url;
#ifdef Q_OS_WIN
    //fix windows "f:/hg/zmq" -> scheme="f" path="/hg/zmq"
    if (url.scheme().length() == 1) {
        QString path = QDir::fromNativeSeparators(url.toString());
        QFileInfo info(path);
        url.setScheme("");
        url.setPath(path);
        if (info.exists()) {
            if (info.isFile()) {
                url.setScheme("file");
            } else if (info.isDir()) {
                url.setScheme("pdoc");
            }
        }
        return url;
    }
#endif
    if (!url.scheme().isEmpty()) {
        return url;
    }
    if (url.isRelative() && !url.path().isEmpty()) {
        if (url.path().compare("/src/pkg/") == 0 || url.path().compare("/pkg/") == 0) {
            url.setScheme("list");
            url.setPath("pkg");
        } else if (url.path().compare("/src/cmd/") == 0  || url.path().compare("/cmd/") == 0){
            url.setScheme("list");
            url.setPath("cmd");
        } else if (url.path().indexOf("/pkg/") == 0) {
            url.setScheme("pdoc");
            if (url.path().at(url.path().length()-1) == '/') {
                url.setPath(url.path().mid(5,url.path().length()-6));
            } else {
                url.setPath(url.path().right(url.path().length()-5));
            }
        } else if (url.path().indexOf("/cmd/") == 0) {
            url.setScheme("pdoc");
            if (url.path().at(url.path().length()-1) == '/') {
                url.setPath(url.path().mid(5,url.path().length()-6));
            } else {
                url.setPath(url.path().right(url.path().length()-5));
            }
        } else if (url.path() == "..") {
            // check ".." is root
            if (m_lastUrl.scheme() == "pdoc") {
                QString path = QDir::cleanPath(m_lastUrl.path()+"/"+url.path());
                if (path != "..") {
                    url.setScheme("pdoc");
                    url.setPath(path);
                }
            }
        } else if (url.path().indexOf("/target/") == 0 && m_lastUrl.scheme() == "pdoc") {
            QString name = url.path().right(url.path().length()-8);
            foreach (QString path, m_targetList) {
                QFileInfo info(path,name);
                if (info.exists()) {
                    url.setScheme("file");
                    url.setPath(info.filePath());
                    break;
                }
            }
        } else {
            QFileInfo info;
            info.setFile(url.path());
            if (!info.exists()) {
                info.setFile(url.toLocalFile());
            }
            if (!info.exists()) {
                QString path = url.path();
                if (path.at(0) == '/') {
                    info.setFile(QDir(m_goroot),path.right(path.length()-1));
                } else if (m_lastUrl.scheme() == "file") {
                    info.setFile(QFileInfo(m_lastUrl.toLocalFile()).absoluteDir(),path);
                }
                if (!info.exists()) {
                    //fix godoc path="f:/hg/zmq/gozmq" "href=/gozmq/gozmq.go"
                    if (m_lastUrl.scheme() == "pdoc") {
                        QDir dir(m_lastUrl.path());
                        if (dir.exists()) {
                            if (path.indexOf(dir.dirName()) == 1) {
                                path.remove(0,dir.dirName().length()+2);
                            }
                            info.setFile(dir,path);
                        }
                    }
                }
            }
            //check index.html
            if (info.exists() && info.isDir()) {
                QFileInfo test(info.dir(),"index.html");
                if (test.exists()) {
                    info = test;
                }
            }

            if (info.exists() && info.isFile()) {
                url.setScheme("file");
                url.setPath(QDir::cleanPath(info.filePath()));
            } else if(url.path().at(url.path().length()-1) != '/') {
                url.setScheme("pdoc");
                if (info.exists()) {
                    url.setPath(info.filePath());
                } else {
                    if (m_lastUrl.scheme() == "pdoc") {
                        url.setPath(QDir::cleanPath(m_lastUrl.path()+"/"+url.path()));
                    } else {
                        url.setPath(url.path());
                    }
                }
            }
        }
    }
    return url;
}

void GolangDoc::highlighted(const QUrl &_url)
{
    if (_url.isEmpty()) {
        m_docBrowser->statusBar()->showMessage(QString("GOROOT=%1").arg(m_goroot));
    } else {
        QUrl url = parserUrl(_url);
        m_docBrowser->statusBar()->showMessage(QString("%1 [%2]").arg(_url.toString()).arg(url.toString()));
    }
}

void GolangDoc::openUrl(const QUrl &_url)
{ 
    QUrl url = parserUrl(_url);
    if ( (m_openUrl.scheme() == url.scheme()) &&
         m_openUrl.path() == url.path()) {
        m_docBrowser->scrollToAnchor(url.fragment());
        m_openUrl = url;
        return;
    }
    m_openUrl = url;
    if (url.scheme() == "find") {
        openUrlFind(url);
    } else if (url.scheme() == "pdoc") {
        openUrlPdoc(url);
    } else if (url.scheme() == "list") {
        openUrlList(url);
    } else if (url.scheme() == "file") {
        openUrlFile(url);
    }
}

void GolangDoc::doubleClickListView(QModelIndex index)
{
    if (!index.isValid()) {
        return;
    }
    QModelIndex src =  m_findFilterModel->mapToSource(index);
    if (!src.isValid()) {
        return;
    }
    QString text = m_findResultModel->data(src,Qt::DisplayRole).toString();
    if (!text.isEmpty()){
        activeBrowser();
        int n = text.indexOf(".");
        if (n >= 0) {
            text.replace(n,1,"#");
        }
        QUrl url(QString("pdoc:%1").arg(text));
        openUrl(url);
    }
}
