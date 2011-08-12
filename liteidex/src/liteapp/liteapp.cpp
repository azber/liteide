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
// Module: liteapp.cpp
// Creator: visualfc <visualfc@gmail.com>
// date: 2011-3-26
// $Id: liteapp.cpp,v 1.0 2011-5-12 visualfc Exp $

#include "liteapp.h"
#include "filemanager.h"
#include "editormanager.h"
#include "projectmanager.h"
#include "pluginmanager.h"
#include "dockmanager.h"
#include "outputmanager.h"
#include "actionmanager.h"
#include "mimetypemanager.h"
#include "optionmanager.h"
#include "mainwindow.h"
#include "liteappoptionfactory.h"

#include <QApplication>
#include <QMenuBar>
#include <QDir>
#include <QToolBar>
#include <QAction>
#include <QDateTime>
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

LiteApp::LiteApp()
    : m_settings(new QSettings(QSettings::IniFormat,QSettings::UserScope,"liteide","liteide",this)),
      m_extension(new Extension),
      m_mainwindow(new MainWindow(this)),
      m_actionManager(new ActionManager),
      m_pluginManager(new PluginManager),
      m_projectManager(new ProjectManager),
      m_editorManager(new EditorManager),
      m_fileManager(new FileManager),
      m_dockManager(new DockManager),
      m_outputManager(new OutputManager),
      m_mimeTypeManager(new MimeTypeManager),
      m_optionManager(new OptionManager)
{
    m_mimeTypeManager->initWithApp(this);
    m_pluginManager->initWithApp(this);
    m_projectManager->initWithApp(this);
    m_editorManager->initWithApp(this);
    m_fileManager->initWithApp(this);
    m_actionManager->initWithApp(this);
    m_dockManager->initWithApp(this);
    m_outputManager->initWithApp(this);
    m_optionManager->initWithApp(this);
    m_mainwindow->init();

    m_extension->addObject("LiteApi.IMimeTypeManager",m_mimeTypeManager);
    m_extension->addObject("LiteApi.IPluginManager",m_pluginManager);
    m_extension->addObject("LiteApi.IProjectManager",m_projectManager);
    m_extension->addObject("LiteApi.IEditManager",m_editorManager);
    m_extension->addObject("LiteApi.IDockManager",m_dockManager);
    m_extension->addObject("LiteApi.IOutputManager",m_outputManager);
    m_extension->addObject("LiteApi.IOptoinManager",m_optionManager);
    m_extension->addObject("LiteApi.QMainWindow",m_mainwindow);

    //add actions
    connect(m_projectManager,SIGNAL(currentProjectChanged(LiteApi::IProject*)),this,SLOT(currentProjectChanged(LiteApi::IProject*)));
    connect(m_editorManager,SIGNAL(currentEditorChanged(LiteApi::IEditor*)),m_mainwindow,SLOT(currentEditorChanged(LiteApi::IEditor*)));
    connect(m_editorManager,SIGNAL(currentEditorChanged(LiteApi::IEditor*)),this,SLOT(currentEditorChanged(LiteApi::IEditor*)));
    connect(m_editorManager,SIGNAL(tabAddRequest()),m_fileManager,SLOT(openEditors()));
    connect(m_editorManager,SIGNAL(editorSaved(LiteApi::IEditor*)),m_fileManager,SLOT(editorSaved(LiteApi::IEditor*)));
    connect(m_editorManager,SIGNAL(editorCreated(LiteApi::IEditor*)),m_fileManager,SLOT(editorCreated(LiteApi::IEditor*)));
    connect(m_editorManager,SIGNAL(editorAboutToClose(LiteApi::IEditor*)),m_fileManager,SLOT(editorAboutToClose(LiteApi::IEditor*)));

    createActions();
    createMenus();
    createToolBars();

    m_extension->addObject("LiteApi.NavToolBar",m_navToolBar);

    m_viewMenu->addAction(m_stdToolBar->toggleViewAction());
    m_viewMenu->addAction(m_navToolBar->toggleViewAction());
    m_viewMenu->addSeparator();

    m_logOutput = new TextOutput;
    m_outputManager->addOutuput(m_logOutput,tr("Console"));
    connect(m_logOutput,SIGNAL(hideOutput()),m_outputManager,SLOT(setCurrentOutput()));

    m_optionAct = m_editorManager->registerBrowser(m_optionManager->browser());
    m_viewMenu->addAction(m_optionAct);
    m_optionManager->setAction(m_optionAct);

    currentProjectChanged(0);
    currentEditorChanged(0);

    this->appendConsole("LiteApp","Init");
    this->appendConsole("LiteApp","ObjectIdList",m_extension->objectMetaList().join(";"));

    m_liteAppOptionFactory = new LiteAppOptionFactory(this,this);

    m_optionManager->addFactory(m_liteAppOptionFactory);
}

LiteApp::~LiteApp()
{
    cleanup();
}

IExtension *LiteApp::extension()
{
    return m_extension;
}

void LiteApp::cleanup()
{
    delete m_liteAppOptionFactory;
    delete m_projectManager;
    delete m_editorManager;
    delete m_fileManager;
    delete m_pluginManager;
    delete m_actionManager;
    delete m_dockManager;
    delete m_outputManager;
    delete m_mimeTypeManager;
    delete m_optionManager;
    delete m_extension;
    delete m_logOutput;
    delete m_mainwindow;
    delete m_settings;
}

IEditorManager *LiteApp::editorManager()
{
    return m_editorManager;
}

IFileManager *LiteApp::fileManager()
{
    return m_fileManager;
}

IProjectManager *LiteApp::projectManager()
{
    return m_projectManager;
}

IDockManager *LiteApp::dockManager()
{
    return m_dockManager;
}

IOutputManager  *LiteApp::outputManager()
{
    return m_outputManager;
}

IActionManager  *LiteApp::actionManager()
{
    return m_actionManager;
}

IMimeTypeManager *LiteApp::mimeTypeManager()
{
    return m_mimeTypeManager;
}

IOptionManager  *LiteApp::optionManager()
{
    return m_optionManager;
}

QMainWindow *LiteApp::mainWindow() const
{
    return m_mainwindow;
}

QSettings *LiteApp::settings()
{
    return m_settings;
}

QString LiteApp::resourcePath() const
{
    return m_resourcePath;
}

QString LiteApp::applicationPath() const
{
    return qApp->applicationDirPath();
}

QString LiteApp::pluginPath() const
{
    return m_pluginPath;
}

void LiteApp::setPluginPath(const QString &path)
{
    m_pluginPath = path;
    appendConsole("LiteApp","setPluginPath",path);
}

void LiteApp::setResourcePath(const QString &path)
{
    m_resourcePath = path;
    appendConsole("LiteApp","setResourcePath",path);
}


QList<IPlugin*> LiteApp::pluginList() const
{
    return m_pluginManager->pluginList();
}

void LiteApp::appendConsole(const QString &model, const QString &action, const QString &log)
{
    QDateTime dt = QDateTime::currentDateTime();
    QString text = dt.toString("yyyy-M-d");
    text += QLatin1Char('@');
    text += dt.toString("hh:mm:ss");
    text += QLatin1Char('@');
    text += model;
    text += QLatin1Char('@');
    text += action;
    text += QLatin1Char('@');
    text += log;
    m_logOutput->append(text);
}

void LiteApp::loadPlugins()
{
    m_pluginManager->loadPlugins(m_pluginPath);
}

void LiteApp::loadMimeType()
{
    QDir dir(m_resourcePath);
    if (dir.cd("mimetype")) {
        m_mimeTypeManager->loadMimeTypes(dir.absolutePath());
    }
}

void LiteApp::initPlugins()
{
    QMap<QString,int> idIndexMap;
    QMap<QString,IPlugin*> idPlguinMap;
    foreach(IPlugin *p, m_pluginManager->pluginList()) {
        idIndexMap.insert(p->id(),0);
        idPlguinMap.insert(p->id(),p);
    }

    foreach(IPlugin *p, m_pluginManager->pluginList()) {
        foreach(QString depId, p->dependPluginList()) {
            idIndexMap.insert(depId,idIndexMap.value(depId)-1);
        }
    }
    QMultiMap<int,IPlugin*> deps;
    QMapIterator<QString,int> i(idIndexMap);
    while (i.hasNext()) {
        i.next();
        deps.insertMulti(i.value(),idPlguinMap.value(i.key()));
    }
    QList<int> keys = deps.keys().toSet().toList();
    qSort(keys);
    foreach(int index, keys) {
        foreach(IPlugin *p, deps.values(index)) {
            bool ret = p->initWithApp(this);
            appendConsole("LiteApp","initPlugin",
                          QString("%1 %2").arg(p->id()).arg(ret?"success":"false"));
        }
    }
}

void LiteApp::createActions()
{
    m_newAct = new QAction(QIcon(":/images/new.png"),tr("New"),m_mainwindow);
    m_newAct->setShortcut(QKeySequence::New);
    m_openAct = new QAction(QIcon(":/images/open.png"),tr("Open"),m_mainwindow);
    m_openAct->setShortcut(QKeySequence::Open);
    m_closeAct = new QAction(QIcon(":/images/close.png"),tr("Close"),m_mainwindow);
    m_closeAct->setShortcut(QKeySequence("CTRL+W"));
    m_closeAllAct = new QAction(QIcon(":/images/closeall.png"),tr("Close All"),m_mainwindow);
    m_openProjectAct = new QAction(QIcon(":/images/openproject.png"),tr("Open Project"),m_mainwindow);
    m_saveProjectAct = new QAction(QIcon(":/images/saveproject.png"),tr("Save Project"),m_mainwindow);
    m_closeProjectAct = new QAction(QIcon(":/images/closeproject.png"),tr("Close Project"),m_mainwindow);
    m_saveAct = new QAction(QIcon(":/images/save.png"),tr("Save"),m_mainwindow);
    m_saveAct->setShortcut(QKeySequence::Save);
    m_saveAsAct = new QAction(tr("Save As..."),m_mainwindow);
    m_saveAsAct->setShortcut(QKeySequence::SaveAs);
    m_saveAllAct = new QAction(QIcon(":/images/saveall.png"),tr("Save All"),m_mainwindow);

    m_exitAct = new QAction(tr("Exit"),m_mainwindow);
    m_exitAct->setShortcut(QKeySequence::Quit);

    m_aboutAct = new QAction(tr("About LiteIDE..."),m_mainwindow);
    m_aboutPluginsAct = new QAction(tr("About Plugins..."),m_mainwindow);

    connect(m_newAct,SIGNAL(triggered()),m_fileManager,SLOT(newFile()));
    connect(m_openAct,SIGNAL(triggered()),m_fileManager,SLOT(openFiles()));
    connect(m_closeAct,SIGNAL(triggered()),m_editorManager,SLOT(closeEditor()));
    connect(m_closeAllAct,SIGNAL(triggered()),m_editorManager,SLOT(closeAllEditors()));
    connect(m_openProjectAct,SIGNAL(triggered()),m_fileManager,SLOT(openProjects()));
    connect(m_saveProjectAct,SIGNAL(triggered()),m_projectManager,SLOT(saveProject()));
    connect(m_closeProjectAct,SIGNAL(triggered()),m_projectManager,SLOT(closeProject()));
    connect(m_saveAct,SIGNAL(triggered()),m_editorManager,SLOT(saveEditor()));
    connect(m_saveAsAct,SIGNAL(triggered()),m_editorManager,SLOT(saveEditorAs()));
    connect(m_saveAllAct,SIGNAL(triggered()),m_editorManager,SLOT(saveAllEditors()));
    connect(m_exitAct,SIGNAL(triggered()),m_mainwindow,SLOT(close()));
    connect(m_aboutAct,SIGNAL(triggered()),m_mainwindow,SLOT(about()));
    connect(m_aboutPluginsAct,SIGNAL(triggered()),m_pluginManager,SLOT(aboutPlugins()));
}

void LiteApp::createMenus()
{
    m_fileMenu = m_actionManager->addMenu("file",tr("&File"));
    m_viewMenu = m_actionManager->addMenu("view",tr("&View"));
    //m_toolMenu = m_actionManager->addMenu("tool",tr("&Tools"));
    m_helpMenu = m_actionManager->addMenu("help",tr("&Help"));

    m_fileMenu->addAction(m_newAct);
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_closeAct);
    m_fileMenu->addAction(m_closeAllAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_openProjectAct);
    m_fileMenu->addAction(m_saveProjectAct);
    m_fileMenu->addAction(m_closeProjectAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);
    m_fileMenu->addAction(m_saveAllAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addMenu(m_fileManager->recentFileMenu());
    m_fileMenu->addMenu(m_fileManager->recentProjectMenu());
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);

    m_helpMenu->addAction(m_aboutAct);
    m_helpMenu->addAction(m_aboutPluginsAct);
}

void LiteApp::createToolBars()
{
    m_stdToolBar = m_mainwindow->addToolBar(tr("Standard ToolBar"));
    m_stdToolBar->setObjectName("StandardToolBar");
    m_stdToolBar->addAction(m_newAct);
    m_stdToolBar->addSeparator();
    m_stdToolBar->addAction(m_openAct);
    m_stdToolBar->addAction(m_saveAct);
    m_stdToolBar->addAction(m_saveAllAct);
    m_stdToolBar->addSeparator();
    m_stdToolBar->addAction(m_openProjectAct);
    m_stdToolBar->addAction(m_saveProjectAct);
    m_stdToolBar->addAction(m_closeProjectAct);

    m_navToolBar = m_mainwindow->addToolBar(tr("Navigation ToolBar"));
    m_navToolBar->setObjectName("NavigationToolBar");
}

void LiteApp::currentProjectChanged(IProject *project)
{
    bool b = (project != 0);
    m_saveProjectAct->setEnabled(b);
    m_closeProjectAct->setEnabled(b);
}

void LiteApp::currentEditorChanged(IEditor *editor)
{
    bool b = (editor != 0);

    if (b) {
        connect(editor,SIGNAL(modificationChanged(bool)),this,SLOT(editorModifyChanged(bool)));
    }
    m_saveAct->setEnabled(b && editor->isModified() && !editor->isReadOnly());
    m_saveAsAct->setEnabled(editor && !editor->fileName().isEmpty());
    m_saveAllAct->setEnabled(b);
    m_closeAct->setEnabled(b);
    m_closeAllAct->setEnabled(b);
}

void LiteApp::editorModifyChanged(bool /*b*/)
{
    IEditor *editor = (IEditor*)sender();
    if (editor && editor->isModified() && !editor->isReadOnly()) {
        m_saveAct->setEnabled(true);
    } else {
        m_saveAct->setEnabled(false);
    }
}

void LiteApp::loadState()
{
    QByteArray  geometry = m_settings->value("liteapp/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        m_mainwindow->restoreGeometry(geometry);
    } else {
        m_mainwindow->resize(640,480);
    }
    m_mainwindow->restoreState(m_settings->value("liteapp/state").toByteArray());
}

void LiteApp::saveState()
{
    m_settings->setValue("liteapp/geometry",m_mainwindow->saveGeometry());
    m_settings->setValue("liteapp/state",m_mainwindow->saveState());
}


void LiteApp::loadSession(const QString &name)
{
    QString session = "session/"+name;
    QString projectName = m_settings->value(session+"_project").toString();
    QString editorName = m_settings->value(session+"_cureditor").toString();
    QStringList fileList = m_settings->value(session+"_alleditor").toStringList();

    if (!projectName.isEmpty()) {
        m_fileManager->openProject(projectName);
    } else {
        m_projectManager->closeProject();
    }

    foreach(QString fileName, fileList) {
        m_fileManager->openEditor(fileName,false);
    }
    if (!editorName.isEmpty()) {
        m_fileManager->openEditor(editorName,true);
    }
}

void LiteApp::saveSession(const QString &name)
{
    QString projectName;
    QString editorName;
    IProject *project = m_projectManager->currentProject();
    if (project) {
        projectName = project->fileName();
    }

    QStringList fileList;
    foreach (IEditor* ed,m_editorManager->sortedEditorList()) {
        if (ed->mimeType().indexOf("liteide/") == 0) {
            continue;
        }
        if (ed->fileName().isEmpty()) {
            continue;
        }
        if (ed == m_editorManager->currentEditor()) {
            editorName = ed->fileName();
        } else {
            fileList.append(ed->fileName());
        }
    }
    QString session = "session/"+name;
    m_settings->setValue(session+"_project",projectName);
    m_settings->setValue(session+"_cureditor",editorName);
    m_settings->setValue(session+"_alleditor",fileList);
}
