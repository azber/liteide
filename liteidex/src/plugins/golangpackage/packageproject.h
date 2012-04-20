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
// Module: packageproject.h
// Creator: visualfc <visualfc@gmail.com>
// date: 2012-4-19
// $Id: packageproject.h,v 1.0 2012-4-19 visualfc Exp $

#ifndef PACKAGEPROJECT_H
#define PACKAGEPROJECT_H

#include "liteapi/liteapi.h"
#include "packagetree.h"

#include <QModelIndex>
#include <QProcess>

class QTreeView;
class FilePathModel;
class QStandardItemModel;
class QStandardItem;
class QDir;
class GoTool;
class PackageProject : public LiteApi::IProject
{
    Q_OBJECT
public:
    enum {
        ITEM_NONE = 0,
        ITEM_SOURCE = 1
    };
    enum {
        RoleItem = Qt::UserRole+1,
        RolePath
    };
public:
    PackageProject(LiteApi::IApplication *app);
    ~PackageProject();
    void setPath(const QString &path);
    void setJson(const QMap<QString,QVariant> &json);
    virtual QWidget *widget();
    virtual QString name() const;
    virtual QString filePath() const;
    virtual QString mimeType() const;
    virtual QStringList fileNameList() const;
    virtual QStringList filePathList() const;
    virtual QString fileNameToFullPath(const QString &filePath);
    virtual QMap<QString,QString> projectInfo() const;
    virtual QMap<QString,QString> targetInfo() const;
    virtual void load();
protected slots:
    void reload();
    void finished(int,QProcess::ExitStatus);
    void doubleClicked(QModelIndex);
    void editorSaved(LiteApi::IEditor*);
protected:
    LiteApi::IApplication *m_liteApp;
    QWidget *m_widget;
    PackageTree *m_treeView;
    QMap<QString,QVariant> m_json;
    GoTool      *m_goTool;
    QTimer      *m_reloadTimer;
};

#endif // PACKAGEPROJECT_H
