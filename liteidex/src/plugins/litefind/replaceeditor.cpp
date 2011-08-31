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
// Module: replaceeditor.cpp
// Creator: visualfc <visualfc@gmail.com>
// date: 2011-8-12
// $Id: replaceeditor.cpp,v 1.0 2011-8-12 visualfc Exp $

#include "replaceeditor.h"
#include "liteapi/litefindobj.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
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

ReplaceEditor::ReplaceEditor(LiteApi::IApplication *app, QObject *parent) :
    QObject(parent),
    m_liteApp(app),
    m_widget(new QWidget)
{
    m_findEdit = new QLineEdit;
    m_replaceEdit = new QLineEdit;

    QPushButton *findNext = new QPushButton(tr("Find Next"));
    QPushButton *replace = new QPushButton(tr("Replace"));
    QPushButton *replaceAll = new QPushButton(tr("Replace All"));
    QPushButton *hideReplace = new QPushButton(tr("Hide"));
    hideReplace->setToolTip(tr("Hide Replace"));

    m_matchWordCheckBox = new QCheckBox(tr("Match whole word only"));
    m_matchCaseCheckBox = new QCheckBox(tr("Match case"));
    m_useRegexCheckBox = new QCheckBox(tr("Regular expression"));
    m_wrapAroundCheckBox = new QCheckBox(tr("Wrap around"));

    m_status = new QLabel(tr("Ready"));
    m_status->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_status->setAlignment(Qt::AlignRight);
    m_status->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    QGridLayout *layout = new QGridLayout;
    layout->setMargin(0);
    layout->setVerticalSpacing(1);

    QHBoxLayout *optLayout = new QHBoxLayout;
    optLayout->setMargin(0);    

    optLayout->addWidget(m_matchWordCheckBox);
    optLayout->addWidget(m_matchCaseCheckBox);
    optLayout->addWidget(m_useRegexCheckBox);
    optLayout->addWidget(m_wrapAroundCheckBox);
    optLayout->addStretch();
    optLayout->addWidget(m_status);

    layout->addWidget(new QLabel(tr("Find What:")),0,0);
    layout->addWidget(m_findEdit,0,1);
    layout->addWidget(findNext,0,2);
    layout->addWidget(hideReplace,0,3);
    layout->addWidget(new QLabel(tr("Replace With:")),1,0);
    layout->addWidget(m_replaceEdit,1,1);
    layout->addWidget(replace,1,2);
    layout->addWidget(replaceAll,1,3);
    layout->addWidget(new QLabel(tr("Find Option:")),3,0);
    layout->addLayout(optLayout,3,1,1,3);

    m_widget->setLayout(layout);

    QWidget::setTabOrder(m_findEdit,m_replaceEdit);


    connect(m_findEdit,SIGNAL(returnPressed()),this,SLOT(findNext()));
    connect(findNext,SIGNAL(clicked()),this,SLOT(findNext()));
    connect(replace,SIGNAL(clicked()),this,SLOT(replace()));
    connect(replaceAll,SIGNAL(clicked()),this,SLOT(replaceAll()));
    connect(hideReplace,SIGNAL(clicked()),this,SIGNAL(hideReplace()));
}

ReplaceEditor::~ReplaceEditor()
{
    if (m_widget) {
        delete m_widget;
    }
}

QWidget *ReplaceEditor::widget()
{
    return m_widget;
}

void ReplaceEditor::setReady(const QString &findText)
{
    m_widget->setFocus();
    if (!findText.isEmpty()) {
        m_findEdit->setText(findText);
    }
    if (m_findEdit->text().isEmpty()) {
        m_findEdit->setFocus();
    } else {
        m_replaceEdit->setFocus();
    }
    m_status->setText(tr("Ready"));
}

void ReplaceEditor::genFindState(ReplaceState *state, bool backWard)
{
    state->findText = m_findEdit->text();
    state->replaceText = m_replaceEdit->text();
    state->matchWord = m_matchWordCheckBox->isChecked();
    state->matchCase = m_matchCaseCheckBox->isChecked();
    state->useRegexp = m_useRegexCheckBox->isChecked();
    state->wrapAround = m_wrapAroundCheckBox->isChecked();
    state->backWard = backWard;
}

QTextCursor ReplaceEditor::findEditor(QTextDocument *doc, const QTextCursor &cursor, ReplaceState *state)
{
    QTextDocument::FindFlags flags = 0;
    if (state->backWard) {
        flags |= QTextDocument::FindBackward;
    }
    if (state->matchCase) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (state->matchWord) {
        flags |= QTextDocument::FindWholeWords;
    }
    QTextCursor find;
    if (state->useRegexp) {
        find = doc->find(QRegExp(state->findText),cursor,flags);
    } else {
        find = doc->find(state->findText,cursor,flags);
    }
    return find;
}


void ReplaceEditor::findNext()
{
    ReplaceState state;
    genFindState(&state,false);
    if (!state.isValid()) {
        return;
    }

    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    if (!editor) {
        return;
    }
    QPlainTextEdit *ed = LiteApi::findExtensionObject<QPlainTextEdit*>(editor,"LiteApi.QPlainTextEdit");
    if (ed) {
        findHelper(ed,&state);
    } else {
        QTextEdit *ed = LiteApi::findExtensionObject<QTextEdit*>(editor,"LiteApi.QTextEdit");
        if (ed) {
            findHelper(ed,&state);
        }
    }
}

void ReplaceEditor::replace()
{
    ReplaceState state;
    genFindState(&state,false);
    if (!state.isValid()) {
        return;
    }

    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    if (!editor) {
        return;
    }
    QPlainTextEdit *ed = LiteApi::findExtensionObject<QPlainTextEdit*>(editor,"LiteApi.QPlainTextEdit");
    if (ed && !ed->isReadOnly()) {
        replaceHelper(ed,&state,1);
    } else {
        QTextEdit *ed = LiteApi::findExtensionObject<QTextEdit*>(editor,"LiteApi.QTextEdit");
        if (ed && !ed->isReadOnly()) {
            replaceHelper(ed,&state,1);
        }
    }
}

void ReplaceEditor::replaceAll()
{
    ReplaceState state;
    genFindState(&state,false);
    if (!state.isValid()) {
        return;
    }

    LiteApi::IEditor *editor = m_liteApp->editorManager()->currentEditor();
    if (!editor) {
        return;
    }
    QPlainTextEdit *ed = LiteApi::findExtensionObject<QPlainTextEdit*>(editor,"LiteApi.QPlainTextEdit");
    if (ed && !ed->isReadOnly()) {
        replaceHelper(ed,&state,-1);
    } else {
        QTextEdit *ed = LiteApi::findExtensionObject<QTextEdit*>(editor,"LiteApi.QTextEdit");
        if (ed && !ed->isReadOnly()) {
            replaceHelper(ed,&state,-1);
        }
    }
}
