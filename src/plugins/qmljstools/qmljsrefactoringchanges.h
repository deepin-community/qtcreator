// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "qmljstools_global.h"

#include <qmljs/qmljsdocument.h>

#include <texteditor/refactoringchanges.h>

namespace QmlJS { class ModelManagerInterface; }

namespace QmlJSTools {

class QmlJSRefactoringChanges;
class QmlJSRefactoringFile;
class QmlJSRefactoringChangesData;
using QmlJSRefactoringFilePtr = QSharedPointer<QmlJSRefactoringFile>;

class QMLJSTOOLS_EXPORT QmlJSRefactoringFile: public TextEditor::RefactoringFile
{
public:
    QmlJS::Document::Ptr qmljsDocument() const;
    QString qmlImports() const;

    /*!
        Returns the offset in the document for the start position of the given
                 source location.
     */
    unsigned startOf(const QmlJS::SourceLocation &loc) const;

    bool isCursorOn(QmlJS::AST::UiObjectMember *ast) const;
    bool isCursorOn(QmlJS::AST::UiQualifiedId *ast) const;
    bool isCursorOn(QmlJS::SourceLocation loc) const;

private:
    QmlJSRefactoringFile(const Utils::FilePath &filePath,
                         const QSharedPointer<QmlJSRefactoringChangesData> &data);
    QmlJSRefactoringFile(TextEditor::TextEditorWidget *editor, QmlJS::Document::Ptr document);

    void fileChanged() override;
    Utils::Id indenterId() const override;

    mutable QmlJS::Document::Ptr m_qmljsDocument;
    QSharedPointer<QmlJSRefactoringChangesData> m_data;

    friend class QmlJSRefactoringChanges;
};


class QMLJSTOOLS_EXPORT QmlJSRefactoringChanges: public TextEditor::RefactoringFileFactory
{
public:
    QmlJSRefactoringChanges(QmlJS::ModelManagerInterface *modelManager,
                            const QmlJS::Snapshot &snapshot);

    static QmlJSRefactoringFilePtr file(TextEditor::TextEditorWidget *editor,
                                        const QmlJS::Document::Ptr &document);
    TextEditor::RefactoringFilePtr file(const Utils::FilePath &filePath) const;

    QmlJSRefactoringFilePtr qmlJSFile(const Utils::FilePath &filePath) const;

    const QmlJS::Snapshot &snapshot() const;

private:
    const QSharedPointer<QmlJSRefactoringChangesData> m_data;
};

} // namespace QmlJSTools
