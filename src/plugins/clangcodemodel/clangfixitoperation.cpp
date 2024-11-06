// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "clangfixitoperation.h"

#include <cppeditor/cppmodelmanager.h>
#include <cppeditor/cpprefactoringchanges.h>

#include <utils/qtcassert.h>

#include <QTextDocument>

namespace ClangCodeModel {
namespace Internal {

using FileToFixits = QMap<Utils::FilePath, QList<ClangFixIt>>;
using RefactoringFilePtr = QSharedPointer<TextEditor::RefactoringFile>;

ClangFixItOperation::ClangFixItOperation(const QString &fixItText, const QList<ClangFixIt> &fixIts)
    : fixItText(fixItText), fixIts(fixIts)
{
}

int ClangFixItOperation::priority() const
{
    return 10;
}

QString ClangFixItOperation::description() const
{
    return QStringLiteral("Apply Fix: ") + fixItText;
}

static FileToFixits fixitsPerFile(const QList<ClangFixIt> &fixIts)
{
    FileToFixits mapping;

    for (const auto &fixItContainer : fixIts) {
        const Utils::FilePath &rangeStartFilePath = fixItContainer.range.start.targetFilePath;
        const Utils::FilePath &rangeEndFilePath = fixItContainer.range.end.targetFilePath;
        QTC_CHECK(rangeStartFilePath == rangeEndFilePath);
        mapping[rangeStartFilePath].append(fixItContainer);
    }

    return mapping;
}

void ClangFixItOperation::perform()
{
    const CppEditor::CppRefactoringChanges refactoringChanges(
        CppEditor::CppModelManager::snapshot());
    const FileToFixits fileToFixIts = fixitsPerFile(fixIts);

    for (auto i = fileToFixIts.cbegin(), end = fileToFixIts.cend(); i != end; ++i) {
        const Utils::FilePath &filePath = i.key();
        const QList<ClangFixIt> &fixits = i.value();

        RefactoringFilePtr refactoringFile = refactoringChanges.file(filePath);
        refactoringFiles.append(refactoringFile);

        applyFixitsToFile(*refactoringFile, fixits);
    }
}

QString ClangFixItOperation::firstRefactoringFileContent_forTestOnly() const
{
    return refactoringFiles.first()->document()->toPlainText();
}

void ClangFixItOperation::applyFixitsToFile(TextEditor::RefactoringFile &refactoringFile,
        const QList<ClangFixIt> fixIts)
{
    refactoringFile.apply(toChangeSet(refactoringFile, fixIts));
}

Utils::ChangeSet ClangFixItOperation::toChangeSet(
        TextEditor::RefactoringFile &refactoringFile,
        const QList<ClangFixIt> fixIts) const
{
    Utils::ChangeSet changeSet;

    for (const auto &fixItContainer : fixIts) {
        const auto &range = fixItContainer.range;
        const auto &start = range.start;
        const auto &end = range.end;
        changeSet.replace(refactoringFile.position(start.targetLine, start.targetColumn + 1),
                          refactoringFile.position(end.targetLine, end.targetColumn + 1),
                          fixItContainer.text);
    }

    return changeSet;
}

} // namespace Internal
} // namespace ClangCodeModel
