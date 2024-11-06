// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "vcsbasediffeditorcontroller.h"

#include <utils/async.h>
#include <utils/environment.h>
#include <utils/qtcprocess.h>

using namespace DiffEditor;
using namespace Tasking;
using namespace Utils;

namespace VcsBase {

class VcsBaseDiffEditorControllerPrivate
{
public:
    VcsBaseDiffEditorControllerPrivate(VcsBaseDiffEditorController *q) : q(q) {}

    VcsBaseDiffEditorController *q;
    Environment m_processEnvironment;
    FilePath m_vcsBinary;
};

/////////////////////

VcsBaseDiffEditorController::VcsBaseDiffEditorController(Core::IDocument *document)
    : DiffEditorController(document)
    , d(new VcsBaseDiffEditorControllerPrivate(this))
{}

VcsBaseDiffEditorController::~VcsBaseDiffEditorController()
{
    delete d;
}

GroupItem VcsBaseDiffEditorController::postProcessTask(const Storage<QString> &inputStorage)
{
    const auto onSetup = [inputStorage](Async<QList<FileData>> &async) {
        async.setConcurrentCallData(&DiffUtils::readPatchWithPromise, *inputStorage);
    };
    const auto onDone = [this](const Async<QList<FileData>> &async, DoneWith result) {
        setDiffFiles(result == DoneWith::Success && async.isResultAvailable()
                     ? async.result() : QList<FileData>());
        // TODO: We should set the right starting line here
    };
    return AsyncTask<QList<FileData>>(onSetup, onDone);
}

void VcsBaseDiffEditorController::setupCommand(Process &process, const QStringList &args) const
{
    process.setEnvironment(d->m_processEnvironment);
    process.setWorkingDirectory(workingDirectory());
    process.setCommand({d->m_vcsBinary, args});
    process.setUseCtrlCStub(true);
}

void VcsBaseDiffEditorController::setVcsBinary(const FilePath &path)
{
    d->m_vcsBinary = path;
}

void VcsBaseDiffEditorController::setProcessEnvironment(const Environment &value)
{
    d->m_processEnvironment = value;
}

} // namespace VcsBase
