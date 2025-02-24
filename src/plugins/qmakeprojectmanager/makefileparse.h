// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <qmakeprojectmanager/qmakestep.h>
#include <qtsupport/baseqtversion.h>
#include <utils/filepath.h>

#include <optional>

namespace QmakeProjectManager::Internal {

struct QMakeAssignment
{
    QString variable;
    QString op;
    QString value;
};

class MakeFileParse
{
public:
    enum class Mode { FilterKnownConfigValues, DoNotFilterKnownConfigValues };
    MakeFileParse(const Utils::FilePath &makefile, Mode mode,
                  std::optional<Utils::FilePath> projectFile = std::nullopt);

    enum MakefileState { MakefileMissing, CouldNotParse, Okay };

    MakefileState makeFileState() const;
    Utils::FilePath qmakePath() const;
    Utils::FilePath srcProFile() const;
    Utils::FilePath qtConfPath() const { return m_qtConfFile;}
    QMakeStepConfig config() const;

    QString unparsedArguments() const;

    QtSupport::QtVersion::QmakeBuildConfigs
        effectiveBuildConfig(QtSupport::QtVersion::QmakeBuildConfigs defaultBuildConfig) const;

    static const QLoggingCategory &logging();

    void parseCommandLine(const QString &command, const QString &project);

private:
    void parseArgs(const QString &args, const QString &project,
                   QList<QMakeAssignment> *assignments, QList<QMakeAssignment> *afterAssignments);
    QList<QMakeAssignment> parseAssignments(const QList<QMakeAssignment> &assignments);

    class QmakeBuildConfig
    {
    public:
        bool explicitDebug = false;
        bool explicitRelease = false;
        bool explicitBuildAll = false;
        bool explicitNoBuildAll = false;
    };

    const Mode m_mode;
    MakefileState m_state;
    Utils::FilePath m_qmakePath;
    Utils::FilePath m_srcProFile;
    Utils::FilePath m_qtConfFile;

    QmakeBuildConfig m_qmakeBuildConfig;
    QMakeStepConfig m_config;
    QString m_unparsedArguments;
};

#ifdef WITH_TESTS
QObject *createQmakeMakeFileParserTest();
#endif

} // QmakeProjectManager::Internal
