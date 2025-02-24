// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "cmaketool.h"

#include <utils/environment.h>
#include <utils/filepath.h>

#include <functional>

namespace Utils {
class MacroExpander;
class OutputLineParser;
} // namespace Utils

namespace ProjectExplorer {
class Project;
}

namespace CMakeProjectManager::Internal {

class CMakeBuildSystem;

class BuildDirParameters
{
public:
    BuildDirParameters();
    explicit BuildDirParameters(CMakeBuildSystem *buildSystem);

    bool isValid() const;
    CMakeTool *cmakeTool() const;

    QString projectName;
    ProjectExplorer::Project *project = nullptr;

    Utils::FilePath sourceDirectory;
    Utils::FilePath buildDirectory;
    QString cmakeBuildType;

    Utils::Environment environment;

    Utils::Id cmakeToolId;

    QStringList initialCMakeArguments;
    QStringList configurationChangesArguments;
    QStringList additionalCMakeArguments;

    Utils::MacroExpander* expander = nullptr;

    QList<Utils::OutputLineParser*> outputParsers() const;

private:
    using OutputParserGenerator = std::function<QList<Utils::OutputLineParser*>()>;
    OutputParserGenerator outputParserGenerator;
};

} // CMakeProjectManager::Internal
