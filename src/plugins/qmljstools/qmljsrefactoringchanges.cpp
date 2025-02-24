// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmljsrefactoringchanges.h"

#include "qmljsqtstylecodeformatter.h"
#include "qmljsmodelmanager.h"
#include "qmljsindenter.h"
#include "qmljstoolsconstants.h"

#include <qmljs/parser/qmljsast_p.h>
#include <texteditor/textdocument.h>
#include <texteditor/tabsettings.h>
#include <projectexplorer/editorconfiguration.h>

using namespace QmlJS;

namespace QmlJSTools {

class QmlJSRefactoringChangesData
{
public:
    QmlJSRefactoringChangesData(ModelManagerInterface *modelManager,
                                const Snapshot &snapshot)
        : m_modelManager(modelManager)
        , m_snapshot(snapshot)
    {}

    ModelManagerInterface *m_modelManager;
    Snapshot m_snapshot;
};

QmlJSRefactoringChanges::QmlJSRefactoringChanges(ModelManagerInterface *modelManager,
                                                 const Snapshot &snapshot)
    : m_data(new QmlJSRefactoringChangesData(modelManager, snapshot))
{
}

TextEditor::RefactoringFilePtr QmlJSRefactoringChanges::file(const Utils::FilePath &filePath) const
{
    return qmlJSFile(filePath);
}

QmlJSRefactoringFilePtr QmlJSRefactoringChanges::qmlJSFile(const Utils::FilePath &filePath) const
{
    return QmlJSRefactoringFilePtr(new QmlJSRefactoringFile(filePath, m_data));
}

QmlJSRefactoringFilePtr QmlJSRefactoringChanges::file(
        TextEditor::TextEditorWidget *editor, const Document::Ptr &document)
{
    return QmlJSRefactoringFilePtr(new QmlJSRefactoringFile(editor, document));
}

const Snapshot &QmlJSRefactoringChanges::snapshot() const
{
    return m_data->m_snapshot;
}

QmlJSRefactoringFile::QmlJSRefactoringFile(
    const Utils::FilePath &filePath, const QSharedPointer<QmlJSRefactoringChangesData> &data)
    : RefactoringFile(filePath), m_data(data)
{
    // the RefactoringFile is invalid if its not for a file with qml or js code
    if (ModelManagerInterface::guessLanguageOfFile(filePath) == Dialect::NoLanguage)
        invalidate();
}

QmlJSRefactoringFile::QmlJSRefactoringFile(TextEditor::TextEditorWidget *editor, Document::Ptr document)
    : RefactoringFile(editor)
    , m_qmljsDocument(document)
{
}

Document::Ptr QmlJSRefactoringFile::qmljsDocument() const
{
    if (!m_qmljsDocument) {
        const QString source = document()->toPlainText();
        const Snapshot &snapshot = m_data->m_snapshot;

        Document::MutablePtr newDoc
            = snapshot.documentFromSource(source,
                                          filePath(),
                                          ModelManagerInterface::guessLanguageOfFile(filePath()));
        newDoc->parse();
        m_qmljsDocument = newDoc;
    }

    return m_qmljsDocument;
}

QString QmlJSRefactoringFile::qmlImports() const
{
    QString imports;
    QmlJS::AST::UiProgram *prog = qmljsDocument()->qmlProgram();
    if (prog && prog->headers) {
        const unsigned int start = startOf(prog->headers->firstSourceLocation());
        const unsigned int end = startOf(prog->members->member->firstSourceLocation());
        imports = textOf(start, end);
    }
    return imports;
}

unsigned QmlJSRefactoringFile::startOf(const SourceLocation &loc) const
{
    return position(loc.startLine, loc.startColumn);
}

bool QmlJSRefactoringFile::isCursorOn(AST::UiObjectMember *ast) const
{
    const unsigned pos = cursor().position();

    return ast->firstSourceLocation().begin() <= pos
            && pos <= ast->lastSourceLocation().end();
}

bool QmlJSRefactoringFile::isCursorOn(AST::UiQualifiedId *ast) const
{
    const unsigned pos = cursor().position();

    if (ast->identifierToken.begin() > pos)
        return false;

    AST::UiQualifiedId *last = ast;
    while (last->next)
        last = last->next;

    return pos <= ast->identifierToken.end();
}

bool QmlJSRefactoringFile::isCursorOn(SourceLocation loc) const
{
    const unsigned pos = cursor().position();
    return pos >= loc.begin() && pos <= loc.end();
}

void QmlJSRefactoringFile::fileChanged()
{
    QTC_ASSERT(!filePath().isEmpty(), return);
    m_qmljsDocument.clear();
    m_data->m_modelManager->updateSourceFiles({filePath()}, true);
}

Utils::Id QmlJSRefactoringFile::indenterId() const
{
    return Constants::QML_JS_SETTINGS_ID;
}

} // namespace QmlJSTools
