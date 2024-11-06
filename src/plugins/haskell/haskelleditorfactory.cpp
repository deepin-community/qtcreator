// Copyright (c) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "haskelleditorfactory.h"

#include "haskellconstants.h"
#include "haskellhighlighter.h"
#include "haskellmanager.h"
#include "haskelltr.h"

#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreplugintr.h>

#include <texteditor/textdocument.h>
#include <texteditor/texteditor.h>
#include <texteditor/textindenter.h>

#include <QAction>

using namespace TextEditor;

namespace Haskell::Internal {

static QWidget *createEditorWidget()
{
    auto widget = new TextEditorWidget;
    auto ghciButton = Core::Command::createToolButtonWithShortcutToolTip(Constants::A_RUN_GHCI);
    ghciButton->defaultAction()->setIconText(Tr::tr("GHCi"));
    QObject::connect(ghciButton, &QToolButton::clicked, widget, [widget] {
        openGhci(widget->textDocument()->filePath());
    });
    widget->insertExtraToolBarWidget(TextEditorWidget::Left, ghciButton);
    return widget;
}

class HaskellEditorFactory : public TextEditorFactory
{
public:
    HaskellEditorFactory()
    {
        setId(Constants::C_HASKELLEDITOR_ID);
        setDisplayName(::Core::Tr::tr("Haskell Editor"));
        addMimeType("text/x-haskell");
        setOptionalActionMask(
                    OptionalActions::UnCommentSelection | OptionalActions::FollowSymbolUnderCursor);
        setDocumentCreator([] { return new TextDocument(Constants::C_HASKELLEDITOR_ID); });
        setIndenterCreator([](QTextDocument *doc) { return new TextIndenter(doc); });
        setEditorWidgetCreator(&createEditorWidget);
        setCommentDefinition(Utils::CommentDefinition("--", "{-", "-}"));
        setParenthesesMatchingEnabled(true);
        setMarksVisible(true);
        setSyntaxHighlighterCreator([] { return new HaskellHighlighter(); });
    }
};

void setupHaskellEditor()
{
    static HaskellEditorFactory theHaskellEditorFactory;
}

} // Haskell::Internal
