/**************************************************************************
** This file is part of LiteIDE
**
** Copyright (c) 2011-2013 LiteIDE Team. All rights reserved.
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
// Module: litebuildplugin.cpp
// Creator: visualfc <visualfc@gmail.com>

#include "litebuildplugin.h"
#include "litebuild.h"
#include "litebuildoptionfactory.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileInfo>

//lite_memory_check_begin
#if defined(WIN32) && defined(_MSC_VER) &&  defined(_DEBUG)
     #define _CRTDBG_MAP_ALLOC
     #include <stdlib.h>
     #include <crtdbg.h>
     #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
     #define new DEBUG_NEW
#endif
//lite_memory_check_end


LiteBuildPlugin::LiteBuildPlugin()
{
}

bool LiteBuildPlugin::load(LiteApi::IApplication *app)
{
    m_liteApp = app;
    m_build = new LiteBuild(app,this);
    app->optionManager()->addFactory(new LiteBuildOptionFactory(app,this));

    //execute editor
    QLayout *layout=app->editorManager()->widget()->layout();
    m_executeWidget = new QWidget;
    m_executeWidget->hide();
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(1);
    m_executeWidget->setLayout(hbox);
    m_executeEdit = new QLineEdit;
    m_workLabel = new ElidedLabel("");
    hbox->addWidget(new QLabel(tr("Exec:")));
    hbox->addWidget(m_executeEdit);
    hbox->addWidget(m_workLabel);
    layout->addWidget(m_executeWidget);

    LiteApi::IActionContext *actionContext = m_liteApp->actionManager()->getActionContext(m_build,"Build");
    QAction *execAct = new QAction(tr("Execute"),this);
    actionContext->regAction(execAct,"Execute","Ctrl+,");
    m_liteApp->actionManager()->insertViewMenu(LiteApi::ViewMenuBrowserPos,execAct);

    connect(execAct,SIGNAL(triggered()),this,SLOT(showExecute()));
    connect(m_executeEdit,SIGNAL(returnPressed()),this,SLOT(execute()));
    connect(m_liteApp,SIGNAL(key_escape()),m_executeWidget,SLOT(hide()));
    connect(m_liteApp->editorManager(),SIGNAL(currentEditorChanged(LiteApi::IEditor*)),this,SLOT(currentEditorChanged(LiteApi::IEditor*)));

    return true;
}

void LiteBuildPlugin::showExecute()
{
    m_executeWidget->show();
    m_executeEdit->selectAll();
    m_executeEdit->setFocus();
    QString work = m_liteApp->applicationPath();
    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    LiteApi::ITextEditor *textEditor = LiteApi::getTextEditor(editor);
    if (textEditor) {
        work = QFileInfo(textEditor->filePath()).path();
    }
    m_workLabel->setText(work);
}

void LiteBuildPlugin::execute()
{
    QString text = m_executeEdit->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    m_executeEdit->selectAll();
    QString cmd = text;
    QString args;
    int find = text.indexOf(" ");
    if (find != -1) {
        cmd = text.left(find);
        args = text.right(text.length()-find-1);
    }
    QString work = m_liteApp->applicationPath();
    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    LiteApi::ITextEditor *textEditor = LiteApi::getTextEditor(editor);
    if (textEditor) {
        work = QFileInfo(textEditor->filePath()).path();
    }
    m_build->executeCommand(cmd.trimmed(),args.trimmed(),work);
}

void LiteBuildPlugin::currentEditorChanged(LiteApi::IEditor *)
{
    if (!m_executeWidget->isVisible()) {
        return;
    }
    QString work = m_liteApp->applicationPath();
    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    LiteApi::ITextEditor *textEditor = LiteApi::getTextEditor(editor);
    if (textEditor) {
        work = QFileInfo(textEditor->filePath()).path();
    }
    m_workLabel->setText(work);
}

Q_EXPORT_PLUGIN(PluginFactory)
