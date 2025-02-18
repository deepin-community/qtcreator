// Copyright (C) 2016 Dmitry Savchenko
// Copyright (C) 2016 Vasiliy Sorokin
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "todoitem.h"
#include "settings.h"

#include <projectexplorer/project.h>
#include <coreplugin/editormanager/ieditor.h>

#include <QHash>
#include <QList>

namespace Todo::Internal {

class TodoItemsModel;
class TodoItemsScanner;

class TodoItemsProvider final : public QObject
{
public:
    explicit TodoItemsProvider(QObject *parent = nullptr);
    TodoItemsModel *todoItemsModel();

    void settingsChanged();
    void projectSettingsChanged(ProjectExplorer::Project *project);

private:
    Settings m_settings;
    TodoItemsModel *m_itemsModel;

    // All to-do items are stored here regardless current scanning scope
    QHash<Utils::FilePath, QList<TodoItem> > m_itemsHash;

    // This list contains only those to-do items that are within current scanning scope
    QList<TodoItem> m_itemsList;

    QList<TodoItemsScanner *> m_scanners;

    ProjectExplorer::Project *m_startupProject;

    bool m_shouldUpdateList;

    void setupItemsModel();
    void setupStartupProjectBinding();
    void setupCurrentEditorBinding();
    void setupUpdateListTimer();
    void updateList();
    void createScanners();
    void setItemsListWithinStartupProject();
    void setItemsListWithinSubproject();

    void itemsFetched(const QString &fileName, const QList<TodoItem> &items);
    void startupProjectChanged(ProjectExplorer::Project *project);
    void projectsFilesChanged();
    void currentEditorChanged();
    void updateListTimeoutElapsed();
};

TodoItemsProvider &todoItemsProvider();

void setupTodoItemsProvider(QObject *guard);

} // Todo::Internal
