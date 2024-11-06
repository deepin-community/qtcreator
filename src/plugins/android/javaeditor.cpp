// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "javaeditor.h"
#include "javaindenter.h"
#include "androidconstants.h"

#include <coreplugin/coreplugintr.h>
#include <coreplugin/editormanager/ieditorfactory.h>

#include <texteditor/codeassist/keywordscompletionassist.h>
#include <texteditor/textdocument.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditor.h>

#include <utils/mimeconstants.h>
#include <utils/uncommentselection.h>

namespace Android::Internal {

static TextEditor::TextDocument *createJavaDocument()
{
    auto doc = new TextEditor::TextDocument;
    doc->setId(Constants::JAVA_EDITOR_ID);
    doc->setMimeType(Utils::Constants::JAVA_MIMETYPE);
    doc->setIndenter(createJavaIndenter(doc->document()));
    return doc;
}

class JavaEditorFactory : public TextEditor::TextEditorFactory
{
public:
    JavaEditorFactory()
    {
        static QStringList keywords = {
            "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char", "class", "const",
            "continue", "default", "do", "double", "else", "enum", "extends", "final", "finally",
            "float", "for", "goto", "if", "implements", "import", "instanceof", "int", "interface",
            "long", "native", "new", "package", "private", "protected", "public", "return", "short",
            "static", "strictfp", "super", "switch", "synchronized", "this", "throw", "throws",
            "transient", "try", "void", "volatile", "while"
        };
        setId(Constants::JAVA_EDITOR_ID);
        setDisplayName(::Core::Tr::tr("Java Editor"));
        addMimeType(Utils::Constants::JAVA_MIMETYPE);

        setDocumentCreator(createJavaDocument);
        setUseGenericHighlighter(true);
        setCommentDefinition(Utils::CommentDefinition::CppStyle);
        setOptionalActionMask(TextEditor::OptionalActions::UnCommentSelection);
        setCompletionAssistProvider(new TextEditor::KeywordsCompletionAssistProvider(keywords));
    }
};

void setupJavaEditor()
{
    static JavaEditorFactory theJavaEditorFactory;
}

} // Android::Internal
