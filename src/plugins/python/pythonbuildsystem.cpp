// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "pythonbuildsystem.h"

#include "pythonbuildconfiguration.h"
#include "pythonconstants.h"
#include "pythonkitaspect.h"
#include "pythonproject.h"
#include "pythontr.h"

#include <coreplugin/documentmanager.h>
#include <coreplugin/messagemanager.h>

#include <projectexplorer/target.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <qmljs/qmljsmodelmanagerinterface.h>

#include <utils/algorithm.h>
#include <utils/mimeutils.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace Core;
using namespace ProjectExplorer;
using namespace Utils;

namespace Python::Internal {

static QJsonObject readObjJson(const FilePath &projectFile, QString *errorMessage)
{
    const expected_str<QByteArray> fileContentsResult = projectFile.fileContents();
    if (!fileContentsResult) {
        *errorMessage = fileContentsResult.error();
        return {};
    }

    const QByteArray content = *fileContentsResult;

    // This assumes the project file is formed with only one field called
    // 'files' that has a list associated of the files to include in the project.
    if (content.isEmpty()) {
        *errorMessage = Tr::tr("Unable to read \"%1\": The file is empty.")
                            .arg(projectFile.toUserOutput());
        return QJsonObject();
    }

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(content, &error);
    if (doc.isNull()) {
        const int line = content.left(error.offset).count('\n') + 1;
        *errorMessage = Tr::tr("Unable to parse \"%1\":%2: %3")
                            .arg(projectFile.toUserOutput()).arg(line)
                            .arg(error.errorString());
        return QJsonObject();
    }

    return doc.object();
}

static QStringList readLines(const FilePath &projectFile)
{
    QSet<QString> visited;
    QStringList lines;

    const expected_str<QByteArray> contents = projectFile.fileContents();
    if (contents) {
        QTextStream stream(contents.value());

        while (true) {
            const QString line = stream.readLine();
            if (line.isNull())
                break;
            if (!Utils::insert(visited, line))
                continue;
            lines.append(line);
        }
    }

    return lines;
}

static QStringList readLinesJson(const FilePath &projectFile, QString *errorMessage)
{
    QSet<QString> visited;
    QStringList lines;

    const QJsonObject obj = readObjJson(projectFile, errorMessage);
    const QJsonArray files = obj.value("files").toArray();
    for (const QJsonValue &file : files) {
        const QString fileName = file.toString();
        if (Utils::insert(visited, fileName))
            lines.append(fileName);
    }

    return lines;
}

static QStringList readImportPathsJson(const FilePath &projectFile, QString *errorMessage)
{
    QStringList importPaths;

    const QJsonObject obj = readObjJson(projectFile, errorMessage);
    if (obj.contains("qmlImportPaths")) {
        const QJsonValue dirs = obj.value("qmlImportPaths");
        const QJsonArray dirs_array = dirs.toArray();

        QSet<QString> visited;

        for (const auto &dir : dirs_array)
            visited.insert(dir.toString());

        importPaths.append(Utils::toList(visited));
    }

    return importPaths;
}

PythonBuildSystem::PythonBuildSystem(PythonBuildConfiguration *buildConfig)
    : BuildSystem(buildConfig)
{
    connect(project(),
            &Project::projectFileIsDirty,
            this,
            &PythonBuildSystem::requestDelayedParse);
    m_buildConfig = buildConfig;
    requestParse();
}

PythonBuildSystem::PythonBuildSystem(ProjectExplorer::Target *target)
    : BuildSystem(target)
{
    connect(project(),
            &Project::projectFileIsDirty,
            this,
            &PythonBuildSystem::requestDelayedParse);
    requestParse();
}

bool PythonBuildSystem::supportsAction(Node *context, ProjectAction action, const Node *node) const
{
    if (node->asFileNode())  {
        return action == ProjectAction::Rename
               || action == ProjectAction::RemoveFile;
    }
    if (node->isFolderNodeType() || node->isProjectNodeType()) {
        return action == ProjectAction::AddNewFile
               || action == ProjectAction::RemoveFile
               || action == ProjectAction::AddExistingFile;
    }
    return BuildSystem::supportsAction(context, action, node);
}

static FileType getFileType(const FilePath &f)
{
    if (f.endsWith(".py"))
        return FileType::Source;
    if (f.endsWith(".qrc"))
        return FileType::Resource;
    if (f.endsWith(".ui"))
        return FileType::Form;
    if (f.endsWith(".qml") || f.endsWith(".js"))
        return FileType::QML;
    return Node::fileTypeForFileName(f);
}

void PythonBuildSystem::triggerParsing()
{
    ParseGuard guard = guardParsingRun();
    parse();

    QList<BuildTargetInfo> appTargets;

    auto newRoot = std::make_unique<PythonProjectNode>(projectDirectory());

    FilePath python;
    if (m_buildConfig)
        python = m_buildConfig->python();
    else if (auto kitPython = PythonKitAspect::python(kit()))
        python = kitPython->command;

    const FilePath projectFile = projectFilePath();
    const QString displayName = projectFile.relativePathFrom(projectDirectory()).toUserOutput();
    newRoot->addNestedNode(
        std::make_unique<PythonFileNode>(projectFile, displayName, FileType::Project));

    bool hasQmlFiles = false;

    for (const FileEntry &entry : std::as_const(m_files)) {
        const QString displayName = entry.filePath.relativePathFrom(projectDirectory()).toUserOutput();
        const FileType fileType = getFileType(entry.filePath);

        hasQmlFiles |= fileType == FileType::QML;

        newRoot->addNestedNode(std::make_unique<PythonFileNode>(entry.filePath, displayName, fileType));
        const MimeType mt = mimeTypeForFile(entry.filePath, MimeMatchMode::MatchExtension);
        if (mt.matchesName(Constants::C_PY_MIMETYPE) || mt.matchesName(Constants::C_PY3_MIMETYPE)
            || mt.matchesName(Constants::C_PY_GUI_MIMETYPE)) {
            BuildTargetInfo bti;
            bti.displayName = displayName;
            bti.buildKey = entry.filePath.toString();
            bti.targetFilePath = entry.filePath;
            bti.projectFilePath = projectFile;
            bti.isQtcRunnable = entry.filePath.fileName() == "main.py";
            bti.additionalData = QVariantMap{{"python", python.toSettings()}};
            appTargets.append(bti);
        }
    }
    project()->setProjectLanguage(ProjectExplorer::Constants::QMLJS_LANGUAGE_ID, hasQmlFiles);

    setRootProjectNode(std::move(newRoot));

    setApplicationTargets(appTargets);

    auto modelManager = QmlJS::ModelManagerInterface::instance();
    if (modelManager) {
        const auto hiddenRccFolders = project()->files(Project::HiddenRccFolders);
        auto projectInfo = modelManager->defaultProjectInfoForProject(project(), hiddenRccFolders);

        for (const FileEntry &importPath : std::as_const(m_qmlImportPaths)) {
            if (!importPath.filePath.isEmpty())
                projectInfo.importPaths.maybeInsert(importPath.filePath, QmlJS::Dialect::Qml);
        }

        modelManager->updateProjectInfo(projectInfo, project());
    }

    guard.markAsSuccess();

    emitBuildSystemUpdated();
}

bool PythonBuildSystem::save()
{
    const FilePath filePath = projectFilePath();
    const QStringList rawList = Utils::transform(m_files, &FileEntry::rawEntry);
    const FileChangeBlocker changeGuard(filePath);
    bool result = false;

    QByteArray newContents;

    // New project file
    if (filePath.endsWith(".pyproject")) {
        expected_str<QByteArray> contents = filePath.fileContents();
        if (contents) {
            QJsonDocument doc = QJsonDocument::fromJson(*contents);
            QJsonObject project = doc.object();
            project["files"] = QJsonArray::fromStringList(rawList);
            doc.setObject(project);
            newContents = doc.toJson();
        } else {
            MessageManager::writeDisrupting(contents.error());
        }
    } else { // Old project file
        newContents = rawList.join('\n').toUtf8();
    }

    const expected_str<qint64> writeResult = filePath.writeFileContents(newContents);
    if (writeResult)
        result = true;
    else
        MessageManager::writeDisrupting(writeResult.error());

    return result;
}

bool PythonBuildSystem::addFiles(Node *, const FilePaths &filePaths, FilePaths *)
{
    const FilePath projectDir = projectDirectory();

    auto comp = [](const FileEntry &left, const FileEntry &right) {
        return left.rawEntry < right.rawEntry;
    };

    const bool isSorted = std::is_sorted(m_files.begin(), m_files.end(), comp);

    for (const FilePath &filePath : filePaths) {
        if (!projectDir.isSameDevice(filePath))
            return false;
        m_files.append(FileEntry{filePath.relativePathFrom(projectDir).toString(), filePath});
    }

    if (isSorted)
        std::sort(m_files.begin(), m_files.end(), comp);

    return save();
}

RemovedFilesFromProject PythonBuildSystem::removeFiles(Node *, const FilePaths &filePaths, FilePaths *)
{

    for (const FilePath &filePath : filePaths) {
        Utils::eraseOne(m_files,
                        [filePath](const FileEntry &entry) { return filePath == entry.filePath; });
    }

    return save() ? RemovedFilesFromProject::Ok : RemovedFilesFromProject::Error;
}

bool PythonBuildSystem::deleteFiles(Node *, const FilePaths &)
{
    return true;
}

bool PythonBuildSystem::renameFiles(Node *, const FilePairs &filesToRename, FilePaths *notRenamed)
{
    bool success = true;
    for (const auto &[oldFilePath, newFilePath] : filesToRename) {
        bool found = false;
        for (FileEntry &entry : m_files) {
            if (entry.filePath == oldFilePath) {
                found = true;
                entry.filePath = newFilePath;
                entry.rawEntry = newFilePath.relativeChildPath(projectDirectory()).toString();
                break;
            }
        }
        if (!found) {
            success = false;
            if (notRenamed)
                *notRenamed << oldFilePath;
        }
    }

    if (!save()) {
        if (notRenamed)
            *notRenamed = firstPaths(filesToRename);
        return false;
    }

    return success;
}

void PythonBuildSystem::parse()
{
    m_files.clear();
    m_qmlImportPaths.clear();

    QStringList files;
    QStringList qmlImportPaths;

    const FilePath filePath = projectFilePath();
    // The PySide project file is JSON based
    if (filePath.endsWith(".pyproject")) {
        QString errorMessage;
        files = readLinesJson(filePath, &errorMessage);
        if (!errorMessage.isEmpty())
            MessageManager::writeFlashing(errorMessage);

        errorMessage.clear();
        qmlImportPaths = readImportPathsJson(filePath, &errorMessage);
        if (!errorMessage.isEmpty())
            MessageManager::writeFlashing(errorMessage);
    } else if (filePath.endsWith(".pyqtc")) {
        // To keep compatibility with PyQt we keep the compatibility with plain
        // text files as project files.
        files = readLines(filePath);
    }

    m_files = processEntries(files);
    m_qmlImportPaths = processEntries(qmlImportPaths);
}

/**
 * Expands environment variables in the given \a string when they are written
 * like $$(VARIABLE).
 */
static void expandEnvironmentVariables(const Environment &env, QString &string)
{
    const QRegularExpression candidate("\\$\\$\\((.+)\\)");

    QRegularExpressionMatch match;
    int index = string.indexOf(candidate, 0, &match);
    while (index != -1) {
        const QString value = env.value(match.captured(1));

        string.replace(index, match.capturedLength(), value);
        index += value.length();

        index = string.indexOf(candidate, index, &match);
    }
}

/**
 * Expands environment variables and converts the path from relative to the
 * project to an absolute path for all given raw paths
 */
QList<PythonBuildSystem::FileEntry> PythonBuildSystem::processEntries(
    const QStringList &rawPaths) const
{
    QList<FileEntry> processed;
    const FilePath projectDir = projectDirectory();
    const Environment env = projectDirectory().deviceEnvironment();

    for (const QString &rawPath : rawPaths) {
        FilePath resolvedPath;
        QString path = rawPath.trimmed();
        if (!path.isEmpty()) {
            expandEnvironmentVariables(env, path);
            resolvedPath = projectDir.resolvePath(path);
        }
        processed << FileEntry{rawPath, resolvedPath};
    }
    return processed;
}

} // namespace Internal
