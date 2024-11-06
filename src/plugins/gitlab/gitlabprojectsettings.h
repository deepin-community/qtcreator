// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/id.h>

#include <QDateTime>
#include <QObject>

namespace ProjectExplorer { class Project; }

namespace Utils { class InfoLabel; }

namespace GitLab {

class Project;

class GitLabProjectSettings : public QObject
{
    Q_OBJECT
public:
    explicit GitLabProjectSettings(ProjectExplorer::Project *project);
    Utils::Id currentServer() const { return m_id; }
    void setCurrentServer(const Utils::Id &id) { m_id = id; }
    QString currentServerHost() const { return m_host; }
    void setCurrentServerHost(const QString &server) { m_host = server; }
    void setCurrentProject(const QString &projectName) { m_currentProject = projectName; }
    QString currentProject() const { return m_currentProject; }
    bool isLinked() const { return m_linked; }
    void setLinked(bool linked);
    QDateTime lastRequest() const { return m_lastRequest; }
    void setLastRequest(const QDateTime &lastRequest) { m_lastRequest = lastRequest; }
    ProjectExplorer::Project *project() const { return m_project; }

    static std::tuple<QString, QString, int> remotePartsFromRemote(const QString &remote);

private:
    void load();
    void save();

    ProjectExplorer::Project *m_project = nullptr;
    QString m_host;
    Utils::Id m_id;
    QDateTime m_lastRequest;
    QString m_currentProject;
    bool m_linked = false;
};

void setupGitlabProjectPanel();

} // namespace GitLab
