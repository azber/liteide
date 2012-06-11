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
// Module: liteeditorwidgetbase.h
// Creator: visualfc <visualfc@gmail.com>
// date: 2011-3-26
// $Id: liteeditorwidgetbase.h,v 1.0 2011- 7-26 visualfc Exp $

#ifndef LITEEDITORWIDGETBASE_H
#define LITEEDITORWIDGETBASE_H

#include <QPlainTextEdit>
#include <QTextBlock>
#include "liteeditorapi/liteeditorapi.h"

class LiteEditorWidgetBase : public QPlainTextEdit
{
    Q_OBJECT
public:
    LiteEditorWidgetBase(QWidget *parent = 0);
    virtual ~LiteEditorWidgetBase();
    void initLoadDocument();
    void setTabWidth(int n);
    void setEditorMark(LiteApi::IEditorMark *mark);
public:
    int extraAreaWidth();
    void extraAreaPaintEvent(QPaintEvent *e);
    void extraAreaMouseEvent(QMouseEvent *e);
    void extraAreaLeaveEvent(QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showTip(const QString &tip);
    void hideTip();    
protected slots:
    void editContentsChanged(int,int,int);
    virtual void highlightCurrentLine();
    virtual void slotUpdateExtraAreaWidth();
    virtual void slotModificationChanged(bool);
    virtual void slotUpdateRequest(const QRect &r, int dy);
    virtual void slotCursorPositionChanged();
    virtual void slotUpdateBlockNotify(const QTextBlock &);
    QChar characterAt(int pos) const;
    void handleHomeKey(bool anchor);    
public slots:
    void gotoMatchBrace();
    void gotoLine(int line, int column, bool center);
    void gotoLineStart();
    void gotoLineStartWithSelection();
    void gotoLineEnd();
    void gotoLineEndWithSelection();
    void cutLine();
    void copyLine();
    void deleteLine();
    void gotoPrevBlock();
    void gotoNextBlock();
    void selectBlock();
    bool findPrevBlock(QTextCursor &cursor, int indent, const QString &skip = "//") const;
    bool findNextBlock(QTextCursor &cursor, int indent, const QString &skip = "//") const;
    bool findStartBlock(QTextCursor &cursor, int indent) const;
    bool findEndBlock(QTextCursor &cursor, int indent) const;
public:
    void setAutoIndent(bool b){
        m_autoIndent = b;
    }
    void setAutoBraces0(bool b) {
        m_autoBraces0 = b;
    }
    void setAutoBraces1(bool b) {
        m_autoBraces1 = b;
    }
    void setAutoBraces2(bool b) {
        m_autoBraces2 = b;
    }
    void setAutoBraces3(bool b) {
        m_autoBraces3 = b;
    }
    void setAutoBraces4(bool b) {
        m_autoBraces4 = b;
    }
    void setLineNumberVisible(bool b) {
        m_lineNumbersVisible = b;
        slotUpdateExtraAreaWidth();
    }
    void setMarksVisible(bool b) {
        m_marksVisible = b;
        slotUpdateExtraAreaWidth();
    }
    bool autoIndent() {
        return m_autoIndent;
    }
    bool lineNumberVisible() {
        return m_lineNumbersVisible;
    }
    bool marksVisiable() {
        return m_marksVisible;
    }
protected:
    void maybeSelectLine();
    bool event(QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void indentBlock(QTextBlock block, bool bIndent);
    void indentCursor(QTextCursor cur, bool bIndent);
    void indentText(QTextCursor cur, bool bIndent);
    void indentEnter(QTextCursor cur);
protected:
    QWidget *m_extraArea;
    LiteApi::IEditorMark *m_editorMark;
    bool m_lineNumbersVisible;
    bool m_marksVisible;
    bool m_autoIndent;
    bool m_autoBraces0; //{
    bool m_autoBraces1; //(
    bool m_autoBraces2; //[
    bool m_autoBraces3; //'
    bool m_autoBraces4; //"
    bool m_bLastBraces;
    QChar m_lastBraces;
    int m_lastSaveRevision;
    int m_extraAreaSelectionNumber;
    bool m_contentsChanged;
    bool m_lastCursorChangeWasInteresting;
};

#endif // LITEEDITORWIDGETBASE_H
