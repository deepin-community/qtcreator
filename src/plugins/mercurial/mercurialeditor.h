// Copyright (C) 2016 Brian McGillion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <vcsbase/vcsbaseeditor.h>

#include <QRegularExpression>

namespace Mercurial::Internal {

class MercurialEditorWidget : public VcsBase::VcsBaseEditorWidget
{
public:
    MercurialEditorWidget();

private:
    QString changeUnderCursor(const QTextCursor &cursor) const override;
    VcsBase::BaseAnnotationHighlighterCreator annotationHighlighterCreator() const override;
    QString decorateVersion(const QString &revision) const override;
    QStringList annotationPreviousVersions(const QString &revision) const override;

    const QRegularExpression exactIdentifier12;
    const QRegularExpression exactIdentifier40;
    const QRegularExpression changesetIdentifier40;
};

} // Mercurial::Internal
