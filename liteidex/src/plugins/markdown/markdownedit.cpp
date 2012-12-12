/**************************************************************************
** This file is part of LiteIDE
**
** Copyright (c) 2011-2012 LiteIDE Team. All rights reserved.
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
// Module: markdownedit.cpp
// Creator: visualfc <visualfc@gmail.com>

#include "markdownedit.h"
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QTextCursor>
#include <QTextBlock>
#include <QPlainTextEdit>
#include <QTextDocument>
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

class Separator : public QAction
{
public:
    Separator(QObject *parent) :
        QAction(parent)
    {
        setSeparator(true);
    }
};

MarkdownEdit::MarkdownEdit(LiteApi::IApplication *app, LiteApi::IEditor *editor, QObject *parent) :
    QObject(parent), m_liteApp(app)
{
    m_editor = LiteApi::getTextEditor(editor);
    if (!m_editor) {
        return;
    }
    m_ed = LiteApi::getPlainTextEdit(editor);
    if (!m_ed) {
        return;
    }
    m_ed->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    QAction *h1 = new QAction(QIcon("icon:markdown/images/h1.png"),"H1",this);
    h1->setShortcut(QKeySequence("Ctrl+1"));
    QAction *h2 = new QAction(QIcon("icon:markdown/images/h2.png"),"H2",this);
    h2->setShortcut(QKeySequence("Ctrl+2"));
    QAction *h3 = new QAction(QIcon("icon:markdown/images/h3.png"),"H3",this);
    h3->setShortcut(QKeySequence("Ctrl+3"));
    QAction *h4 = new QAction(QIcon("icon:markdown/images/h4.png"),"H4",this);
    h4->setShortcut(QKeySequence("Ctrl+4"));
    QAction *h5 = new QAction(QIcon("icon:markdown/images/h5.png"),"H5",this);
    h5->setShortcut(QKeySequence("Ctrl+5"));
    QAction *h6 = new QAction(QIcon("icon:markdown/images/h6.png"),"H6",this);
    h6->setShortcut(QKeySequence("Ctrl+6"));

    QAction *bold = new QAction(QIcon("icon:markdown/images/bold.png"),"Bold",this);
    bold->setShortcut(QKeySequence::Bold);
    QAction *italic = new QAction(QIcon("icon:markdown/images/italic.png"),"Italic",this);
    italic->setShortcut(QKeySequence::Italic);
    QAction *code = new QAction(QIcon("icon:markdown/images/code.png"),"Inline Code",this);
    code->setShortcut(QKeySequence("Ctrl+K"));

    QAction *link = new QAction(QIcon("icon:markdown/images/link.png"),"Link",this);
    link->setShortcut(QKeySequence("Ctrl+Shift+L"));

    QAction *image = new QAction(QIcon("icon:markdown/images/image.png"),"Image",this);
    image->setShortcut(QKeySequence("Ctrl+Shift+I"));

    QToolBar *toolBar = LiteApi::findExtensionObject<QToolBar*>(editor,"LiteApi.QToolBar");

    QMenu *menu = LiteApi::getEditMenu(editor);
    if (menu) {
        menu->addSeparator();
        QMenu *h = menu->addMenu("Heading");
        h->addAction(h1);
        h->addAction(h2);
        h->addAction(h3);
        h->addAction(h4);
        h->addAction(h5);
        h->addAction(h6);
        menu->addSeparator();
        menu->addAction(bold);
        menu->addAction(italic);
        menu->addAction(code);
        menu->addSeparator();
        menu->addAction(link);
        menu->addAction(image);
    }
    menu = LiteApi::getContextMenu(editor);
    if (menu) {
        menu->addSeparator();
        menu->addAction(bold);
        menu->addAction(italic);
        menu->addAction(code);
        menu->addSeparator();
        menu->addAction(link);
        menu->addAction(image);
    }

    if (toolBar) {
        toolBar->addSeparator();
        toolBar->addAction(h1);
        toolBar->addAction(h2);
        toolBar->addAction(h3);
        toolBar->addSeparator();
        toolBar->addAction(bold);
        toolBar->addAction(italic);
        toolBar->addAction(code);
        toolBar->addAction(link);
        toolBar->addAction(image);
    }

    connect(editor,SIGNAL(destroyed()),this,SLOT(deleteLater()));
    connect(h1,SIGNAL(triggered()),this,SLOT(h1()));
    connect(h2,SIGNAL(triggered()),this,SLOT(h2()));
    connect(h3,SIGNAL(triggered()),this,SLOT(h3()));
    connect(h4,SIGNAL(triggered()),this,SLOT(h4()));
    connect(h5,SIGNAL(triggered()),this,SLOT(h5()));
    connect(h6,SIGNAL(triggered()),this,SLOT(h6()));
    connect(bold,SIGNAL(triggered()),this,SLOT(bold()));
    connect(italic,SIGNAL(triggered()),this,SLOT(italic()));
    connect(code,SIGNAL(triggered()),this,SLOT(code()));
    connect(link,SIGNAL(triggered()),this,SLOT(link()));
    connect(image,SIGNAL(triggered()),this,SLOT(image()));
}

MarkdownEdit::~MarkdownEdit()
{
}



void MarkdownEdit::insert_head(const QString &tag)
{
    QTextCursor cur = m_ed->textCursor();
    this->gotoLine(cur.block().firstLineNumber(),0);
    cur = m_ed->textCursor();
    cur.insertText(tag+" ");
    m_ed->setTextCursor(cur);
}

void MarkdownEdit::mark_selection(const QString &mark)
{
    QTextCursor cur = m_ed->textCursor();
    cur.beginEditBlock();
    if (cur.hasSelection()) {
        QTextBlock begin = m_ed->document()->findBlock(cur.selectionStart());
        QTextBlock end = m_ed->document()->findBlock(cur.selectionEnd());
        if (end.position() == cur.selectionEnd()) {
            end = end.previous();
        }
        int n1 = cur.selectionStart();
        int n2 = cur.selectionEnd();
        QTextBlock block = begin;
        do {
            int c1 = block.position();
            int c2 = c1+block.text().length();
            if (block.position() == begin.position() && c1 < n1) {
                c1 = n1;
            }
            if (c2 > n2) {
                c2 = n2;
            }
            if (c2 > c1) {
                cur.setPosition(c1);
                cur.insertText(mark);
                cur.setPosition(c2+mark.length());
                cur.insertText(mark);
                n2 += mark.length()*2;
            }
            block = block.next();
        } while(block.isValid() && block.position() <= end.position());
    } else {
        cur.insertText(mark+mark);
        cur.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,mark.length());
    }
    cur.endEditBlock();
    m_ed->setTextCursor(cur);
}

void MarkdownEdit::h1()
{
    insert_head("#");
}

void MarkdownEdit::h2()
{
    insert_head("##");
}

void MarkdownEdit::h3()
{
    insert_head("###");
}

void MarkdownEdit::h4()
{
    insert_head("####");
}

void MarkdownEdit::h5()
{
    insert_head("#####");
}

void MarkdownEdit::h6()
{
    insert_head("######");
}

void MarkdownEdit::bold()
{
    mark_selection("**");
}

void MarkdownEdit::italic()
{
    mark_selection("_");
}

void MarkdownEdit::code()
{
    mark_selection("`");
}

void MarkdownEdit::link()
{
    QTextCursor cursor = m_ed->textCursor();
    cursor.beginEditBlock();
    if (cursor.hasSelection()) {
        int n1 = cursor.selectionStart();
        int n2 = cursor.selectionEnd();
        cursor.setPosition(n1);
        cursor.insertText("[");
        cursor.setPosition(n2+1);
        cursor.insertText("]()");
        cursor.setPosition(n2+3);
    } else {
        int n = cursor.position();
        cursor.insertText("[]()");
        cursor.setPosition(n+1);
    }
    cursor.endEditBlock();
    m_ed->setTextCursor(cursor);
}

void MarkdownEdit::image()
{
    QTextCursor cursor = m_ed->textCursor();
    cursor.beginEditBlock();
    if (cursor.hasSelection()) {
        int n1 = cursor.selectionStart();
        int n2 = cursor.selectionEnd();
        cursor.setPosition(n1);
        cursor.insertText("![");
        cursor.setPosition(n2+2);
        cursor.insertText("]()");
        cursor.setPosition(n2+4);
    } else {
        int n = cursor.position();
        cursor.insertText("![]()");
        cursor.setPosition(n+2);
    }
    cursor.endEditBlock();
    m_ed->setTextCursor(cursor);
}


void MarkdownEdit::gotoLine(int line, int column)
{
    const int blockNumber = line;
    const QTextBlock &block = m_ed->document()->findBlockByLineNumber(blockNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        if (column > 0) {
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
        } else {
            int pos = cursor.position();
            while (m_ed->document()->characterAt(pos).category() == QChar::Separator_Space) {
                ++pos;
            }
            cursor.setPosition(pos);
        }
        m_ed->setTextCursor(cursor);
        m_ed->ensureCursorVisible();
    }
}

