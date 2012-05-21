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
// Module: litefindplugin.h
// Creator: visualfc <visualfc@gmail.com>
// date: 2011-8-12
// $Id: litefindplugin.h,v 1.0 2011-8-12 visualfc Exp $

#ifndef LITEFINDPLUGIN_H
#define LITEFINDPLUGIN_H

#include "litefind_global.h"
#include "liteapi/liteapi.h"
#include <QtPlugin>

class FindEditor;
class ReplaceEditor;
class FileSearch;
class LiteFindPlugin : public LiteApi::IPlugin
{
    Q_OBJECT
    Q_INTERFACES(LiteApi::IPlugin)
public:
    LiteFindPlugin();
    ~LiteFindPlugin();
    virtual bool initWithApp(LiteApi::IApplication *app);
public slots:
    void find(bool);
    void replace(bool);
    void hideFind();
    void hideReplace();
    void switchReplace();
    void fileSearch(bool);
protected:
    FindEditor *m_findEditor;
    ReplaceEditor *m_replaceEditor;
    FileSearch  *m_fileSearch;
    QAction *m_findAct;
    QAction *m_findNextAct;
    QAction *m_findPrevAct;
    QAction *m_replaceAct;
    QAction *m_fileSearchAct;
};

#endif // LITEFINDPLUGIN_H
