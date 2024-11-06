// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "perforceeditor.h"

#include "annotationhighlighter.h"
#include "perforceplugin.h"
#include "perforcetr.h"

#include <coreplugin/editormanager/editormanager.h>
#include <utils/qtcassert.h>
#include <vcsbase/diffandloghighlighter.h>

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMenu>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextStream>

namespace Perforce::Internal {

// ------------ PerforceEditor
PerforceEditorWidget::PerforceEditorWidget() :
    m_changeNumberPattern("^\\d+$")
{
    QTC_CHECK(m_changeNumberPattern.isValid());
    // Diff format:
    // 1) "==== //depot/.../mainwindow.cpp#2 - /depot/.../mainwindow.cpp ====" (created by p4 diff)
    // 2) "==== //depot/.../mainwindow.cpp#15 (text) ====" (created by p4 describe)
    // 3) --- //depot/XXX/closingkit/trunk/source/cui/src/cui_core.cpp<tab>2012-02-08 13:54:01.000000000 0100
    //    +++ P:/XXX\closingkit\trunk\source\cui\src\cui_core.cpp<tab>2012-02-08 13:54:01.000000000 0100
    setDiffFilePattern("^(?:={4}|\\+{3}) (.+)(?:\\t|#\\d)");
    setLogEntryPattern("^... #\\d change (\\d+) ");
    setAnnotateRevisionTextFormat(Tr::tr("Annotate change list \"%1\""));
    setAnnotationEntryPattern("^(\\d+):");
}

QString PerforceEditorWidget::changeUnderCursor(const QTextCursor &c) const
{
    QTextCursor cursor = c;
    // Any number is regarded as change number.
    cursor.select(QTextCursor::WordUnderCursor);
    if (!cursor.hasSelection())
        return {};
    const QString change = cursor.selectedText();
    return m_changeNumberPattern.match(change).hasMatch() ? change : QString();
}

VcsBase::BaseAnnotationHighlighterCreator PerforceEditorWidget::annotationHighlighterCreator() const
{
    return VcsBase::getAnnotationHighlighterCreator<PerforceAnnotationHighlighter>();
}

QString PerforceEditorWidget::findDiffFile(const QString &f) const
{
    return fileNameFromPerforceName(f.trimmed(), false);
}

QStringList PerforceEditorWidget::annotationPreviousVersions(const QString &v) const
{
    bool ok;
    const int changeList = v.toInt(&ok);
    if (!ok || changeList < 2)
        return {};
    return QStringList(QString::number(changeList - 1));
}

} // Perforce::Internal
