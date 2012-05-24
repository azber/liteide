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
// Module: litedebug.cpp
// Creator: visualfc <visualfc@gmail.com>
// date: 2011-8-12
// $Id: litedebug.cpp,v 1.0 2011-8-12 visualfc Exp $

#include "litedebug.h"
#include "debugmanager.h"
#include "debugwidget.h"
#include "liteapi/litefindobj.h"
#include "fileutil/fileutil.h"


#include <QLayout>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QPushButton>
#include <QLabel>
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


LiteDebug::LiteDebug(LiteApi::IApplication *app, QObject *parent) :
    QObject(parent),
    m_liteApp(app),
    m_envManager(0),
    m_liteBuild(0),
    m_debugger(0),
    m_manager(new DebugManager(this)),
    m_widget(new QWidget),
    m_dbgWidget(new DebugWidget(app,this))
{
    m_manager->initWithApp(app);

    m_toolBar =  m_liteApp->actionManager()->insertToolBar("toolbar/litedebug",tr("Debug ToolBar"),"toolbar/nav");
    m_liteApp->actionManager()->insertViewMenu(LiteApi::ViewMenuToolBarPos,m_toolBar->toggleViewAction());

    m_output = new TextOutput;
    m_output->setReadOnly(true);
    m_output->setMaxLine(1024);
    connect(m_output,SIGNAL(hideOutput()),m_liteApp->outputManager(),SLOT(setCurrentOutput()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_dbgWidget->widget());
    m_widget->setLayout(layout);

    m_startDebugAct = new QAction(QIcon(":/images/startdebug.png"),tr("Go"),this);
    m_startDebugAct->setShortcut(QKeySequence(Qt::Key_F5));
    m_startDebugAct->setToolTip(tr("Start Debugging (F5)"));

    m_stopDebugAct = new QAction(QIcon(":/images/stopdebug.png"),tr("Stop"),this);
    m_stopDebugAct->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F5));
    m_stopDebugAct->setToolTip(tr("Stop Debugger (Shift+F5)"));

    m_showLineAct = new QAction(QIcon(":/images/showline.png"),tr("ShowLine"),this);
    m_showLineAct->setToolTip(tr("Show Current Line"));

    m_stepIntoAct = new QAction(QIcon(":/images/stepinto.png"),tr("StepInto"),this);
    m_stepIntoAct->setShortcut(QKeySequence(Qt::Key_F11));
    m_stepIntoAct->setToolTip(tr("Step Info (F11)"));

    m_stepOverAct = new QAction(QIcon(":/images/stepover.png"),tr("StepOver"),this);
    m_stepOverAct->setShortcut(QKeySequence(Qt::Key_F10));
    m_stepOverAct->setToolTip(tr("Step Over (F10)"));

    m_stepOutAct = new QAction(QIcon(":/images/stepout.png"),tr("StepOut"),this);
    m_stepOutAct->setShortcut(QKeySequence(Qt::SHIFT+Qt::Key_F11));
    m_stepOutAct->setToolTip(tr("Step Out (Shift+F11)"));

    m_runToLineAct = new QAction(QIcon(":/images/runtoline.png"),tr("RunToLine"),this);
    m_runToLineAct->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_F10));
    m_runToLineAct->setToolTip(tr("Run to Line (Ctrl+F10)"));

    m_insertBreakAct = new QAction(QIcon(":/images/insertbreak.png"),tr("BreakPoint"),this);
    m_insertBreakAct->setShortcut(QKeySequence(Qt::Key_F9));
    m_insertBreakAct->setToolTip(tr("Insert/Remove Breakpoint (F9)"));

    m_toolBar->addAction(m_startDebugAct);
    m_toolBar->addAction(m_stopDebugAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_showLineAct);
    m_toolBar->addAction(m_stepIntoAct);
    m_toolBar->addAction(m_stepOverAct);
    m_toolBar->addAction(m_stepOutAct);
    m_toolBar->addAction(m_runToLineAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_insertBreakAct);

    QMenu *menu = m_liteApp->actionManager()->insertMenu("Debug",tr("&Debug"),"help");
    if (menu) {
        menu->addAction(m_startDebugAct);
        menu->addAction(m_stopDebugAct);
        menu->addSeparator();
        menu->addAction(m_showLineAct);
        menu->addAction(m_stepIntoAct);
        menu->addAction(m_stepOverAct);
        menu->addAction(m_stepOutAct);
        menu->addAction(m_runToLineAct);
    }

    connect(m_manager,SIGNAL(currentDebuggerChanged(LiteApi::IDebugger*)),this,SLOT(setDebugger(LiteApi::IDebugger*)));
    connect(m_liteApp,SIGNAL(loaded()),this,SLOT(appLoaded()));

    connect(m_startDebugAct,SIGNAL(triggered()),this,SLOT(startDebug()));
    connect(m_runToLineAct,SIGNAL(triggered()),this,SLOT(runToLine()));
    connect(m_stopDebugAct,SIGNAL(triggered()),this,SLOT(stopDebug()));
    connect(m_stepOverAct,SIGNAL(triggered()),this,SLOT(stepOver()));
    connect(m_stepIntoAct,SIGNAL(triggered()),this,SLOT(stepInto()));
    connect(m_stepOutAct,SIGNAL(triggered()),this,SLOT(stepOut()));
    connect(m_insertBreakAct,SIGNAL(triggered()),this,SLOT(toggleBreakPoint()));
    connect(m_showLineAct,SIGNAL(triggered()),this,SLOT(showLine()));
    connect(m_liteApp->editorManager(),SIGNAL(editorCreated(LiteApi::IEditor*)),this,SLOT(editorCreated(LiteApi::IEditor*)));
    connect(m_output,SIGNAL(enterText(QString)),this,SLOT(enterAppInputText(QString)));

    m_liteApp->extension()->addObject("LiteApi.IDebugManager",m_manager);

    m_liteApp->outputManager()->addOutuput(m_output,tr("Debug Output"));

    m_startDebugAct->setToolTip(tr("Start Debugging (F5)"));
    m_stopDebugAct->setEnabled(false);
    m_stepOverAct->setEnabled(false);
    m_showLineAct->setEnabled(false);
    m_stepIntoAct->setEnabled(false);
    m_stepOutAct->setEnabled(false);
    m_runToLineAct->setEnabled(false);
}

LiteDebug::~LiteDebug()
{
    if (m_output) {
        delete m_output;
    }
}

void LiteDebug::appLoaded()
{
    m_liteBuild = LiteApi::findExtensionObject<LiteApi::ILiteBuild*>(m_liteApp,"LiteApi.ILiteBuild");
    m_envManager = LiteApi::findExtensionObject<LiteApi::IEnvManager*>(m_liteApp,"LiteApi.IEnvManager");

    LiteApi::IEditorMarkTypeManager *markTypeManager = LiteApi::findExtensionObject<LiteApi::IEditorMarkTypeManager*>(m_liteApp,"LiteApi.IEditorMarkTypeManager");
    if (markTypeManager) {
        markTypeManager->registerMark(BreakPointMark,QIcon(":/images/breakmark.png"));
        markTypeManager->registerMark(CurrentLineMark,QIcon(":/images/linemark.png"));
    }
}

void LiteDebug::editorCreated(LiteApi::IEditor *editor)
{
    if (!editor) {
        return;
    }
    LiteApi::IEditorMark *editorMark = LiteApi::findExtensionObject<LiteApi::IEditorMark*>(editor,"LiteApi.IEditorMark");
    if (!editorMark) {
        return;
    }
    QList<int> bpList = m_fileBpMap.values(editor->filePath());
    foreach (int line, bpList) {
        editorMark->addMark(line,LiteApi::BreakPointMark);
    }
    if (m_lastLine.fileName == editor->filePath()) {
        editorMark->addMark(m_lastLine.line,LiteApi::CurrentLineMark);
    }
}

QWidget *LiteDebug::widget()
{
    return m_widget;
}

void LiteDebug::setDebugger(LiteApi::IDebugger *debug)
{
    m_debugger = debug;
    if (m_debugger) {
        connect(m_debugger,SIGNAL(debugStarted()),this,SLOT(debugStarted()));
        connect(m_debugger,SIGNAL(debugStoped()),this,SLOT(debugStoped()));
        connect(m_debugger,SIGNAL(debugLog(LiteApi::DEBUG_LOG_TYPE,QString)),this,SLOT(debugLog(LiteApi::DEBUG_LOG_TYPE,QString)));
        connect(m_debugger,SIGNAL(setCurrentLine(QString,int)),this,SLOT(setCurrentLine(QString,int)));
    }
    m_dbgWidget->setDebugger(m_debugger);
}

void LiteDebug::debugLog(LiteApi::DEBUG_LOG_TYPE type, const QString &log)
{
    switch(type) {
    case LiteApi::DebugConsoleLog:
        m_dbgWidget->appendLog(log);
        break;
    case LiteApi::DebugRuntimeLog:
        m_output->appendTag1(QString("<%1>\n").arg(log));
        break;
    case LiteApi::DebugErrorLog:
        m_output->appendTag1(QString("<Error %1>\n").arg(log));
        break;
    case LiteApi::DebugApplationLog:
        m_output->append(log);
        break;
    }
}

void LiteDebug::startDebug()
{
    if (!m_debugger) {
        return;
    }
    if (m_debugger->isRunning()) {
        m_debugger->continueRun();
        return;
    }
    if (!m_liteBuild || !m_liteBuild->buildManager()->currentBuild()) {
        return;
    }
    if (!m_envManager) {
        return;
    }
    m_dbgWidget->clearLog();

    QString targetFilepath = m_liteBuild->targetFilePath();
    if (targetFilepath.isEmpty() || !QFile::exists(targetFilepath)) {
        return;
    }
    QMap<QString,QString> m = m_liteBuild->buildEnvMap();
    QString workDir = m.value("WORKDIR");
    QString target = m.value("TARGETNAME");
    QString args = m.value("TARGETARGS");
    int index = targetFilepath.lastIndexOf(target);
    if (index != -1) {
        target = targetFilepath.right(targetFilepath.length()-index);
    }

    m_debugger->setInitBreakTable(m_fileBpMap);
    m_debugger->setEnvironment(m_envManager->currentEnvironment().toStringList());
    m_debugger->setWorkingDirectory(workDir);
    m_debugger->start(target,args.split(" "));
}

void LiteDebug::continueRun()
{
    if (!m_debugger || !m_debugger->isRunning()) {
        return;
    }
    m_debugger->continueRun();
}

void LiteDebug::runToLine()
{
    if (!m_debugger || !m_debugger->isRunning()) {
        return;
    }
    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    if (!editor) {
        return;
    }
    LiteApi::ITextEditor *textEditor = LiteApi::findExtensionObject<LiteApi::ITextEditor*>(editor,"LiteApi.ITextEditor");
    if (!textEditor) {
        return;
    }
    QString filePath = textEditor->filePath();
    if (filePath.isEmpty()) {
        return;
    }
    QString fileName = QFileInfo(filePath).fileName();
    m_debugger->runToLine(fileName,textEditor->line());
}

void LiteDebug::stopDebug()
{
    if (!m_debugger || !m_debugger->isRunning()) {
        return;
    }
    m_debugger->stop();
}

void LiteDebug::stepOver()
{
    if (!m_debugger || !m_debugger->isRunning()) {
        return;
    }
    m_debugger->stepOver();
}

void LiteDebug::stepInto()
{
    if (!m_debugger || !m_debugger->isRunning()) {
        return;
    }
    m_debugger->stepInto();
}

void LiteDebug::stepOut()
{
    if (!m_debugger || !m_debugger->isRunning()) {
        return;
    }
    m_debugger->stepOut();
}

void LiteDebug::showLine()
{
    if (m_lastLine.fileName.isEmpty()) {
        return;
    }
    LiteApi::IEditor *editor = m_liteApp->fileManager()->openEditor(m_lastLine.fileName,true);
    if (editor) {
        LiteApi::ITextEditor *textEditor = LiteApi::findExtensionObject<LiteApi::ITextEditor*>(editor,"LiteApi.ITextEditor");
        if (textEditor) {
            textEditor->gotoLine(m_lastLine.line,0,true);
        }
    }
}

void LiteDebug::toggleBreakPoint()
{
    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    if (!editor) {
        return;
    }
    LiteApi::IEditorMark *editorMark = LiteApi::findExtensionObject<LiteApi::IEditorMark*>(editor,"LiteApi.IEditorMark");
    if (!editorMark) {
        return;
    }
    LiteApi::ITextEditor *textEditor = LiteApi::findExtensionObject<LiteApi::ITextEditor*>(editor,"LiteApi.ITextEditor");
    if (!textEditor) {
        return;
    }
    int line = textEditor->line();
    QString fileName = editor->filePath();
    if (fileName.isEmpty()) {
        return;
    }
    QList<int> marks = editorMark->lineTypeList(line);
    if (marks.contains(LiteApi::BreakPointMark)) {
        editorMark->removeMark(line,LiteApi::BreakPointMark);
        m_fileBpMap.remove(fileName,line);
        if (m_debugger && m_debugger->isRunning()) {
            m_debugger->removeBreakPoint(fileName,line);
        }
    } else {
        editorMark->addMark(line,LiteApi::BreakPointMark);
        m_fileBpMap.insert(fileName,line);
        if (m_debugger && m_debugger->isRunning()) {
            m_debugger->insertBreakPoint(fileName,line);
        }
    }
}

void LiteDebug::clearLastLine()
{
    if (!m_lastLine.fileName.isEmpty()) {
        LiteApi::IEditor *lastEditor = m_liteApp->editorManager()->findEditor(m_lastLine.fileName,true);
        if (lastEditor) {
            LiteApi::IEditorMark *lastMark = LiteApi::findExtensionObject<LiteApi::IEditorMark*>(lastEditor,"LiteApi.IEditorMark");
            if (lastMark) {
                lastMark->removeMark(m_lastLine.line,LiteApi::CurrentLineMark);
            }
        }
    }
    m_lastLine.fileName.clear();
}

void LiteDebug::debugStarted()
{
    m_startDebugAct->setToolTip(tr("Continue (F5)"));
    m_stopDebugAct->setEnabled(true);
    m_showLineAct->setEnabled(true);
    m_stepOverAct->setEnabled(true);
    m_stepIntoAct->setEnabled(true);
    m_stepOutAct->setEnabled(true);
    m_runToLineAct->setEnabled(true);
    m_output->setReadOnly(false);
    m_liteApp->outputManager()->setCurrentOutput(m_output);
    m_widget->show();    
    emit debugVisible(true);
}

void LiteDebug::debugStoped()
{
    m_startDebugAct->setToolTip(tr("Start Debugging (F5)"));
    m_stopDebugAct->setEnabled(false);
    m_stepOverAct->setEnabled(false);
    m_showLineAct->setEnabled(false);
    m_stepIntoAct->setEnabled(false);
    m_stepOutAct->setEnabled(false);
    m_runToLineAct->setEnabled(false);
    clearLastLine();
    m_output->setReadOnly(true);
    m_liteApp->outputManager()->setCurrentOutput(m_output);
    m_widget->hide();
    emit debugVisible(false);
}

void LiteDebug::setCurrentLine(const QString &fileName, int line)
{
    bool center = true;
    if (m_lastLine.fileName == fileName) {
        center = false;
    }
    clearLastLine();
    if (QFile::exists(fileName)) {
        LiteApi::IEditor *editor = m_liteApp->fileManager()->openEditor(fileName,true);
        if (editor) {
            m_lastLine.fileName = fileName;
            m_lastLine.line = line;
            LiteApi::ITextEditor *textEditor = LiteApi::findExtensionObject<LiteApi::ITextEditor*>(editor,"LiteApi.ITextEditor");
            if (textEditor) {
                textEditor->gotoLine(line,0,center);
            }
            LiteApi::IEditorMark *editMark = LiteApi::findExtensionObject<LiteApi::IEditorMark*>(editor,"LiteApi.IEditorMark");
            if (editMark) {
                editMark->addMark(line,LiteApi::CurrentLineMark);
            }
        }
    }
}

void LiteDebug::enterAppInputText(QString text)
{
    if (m_debugger && m_debugger->isRunning()) {
        m_debugger->enterText(text);
    }
}

