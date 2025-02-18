// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qbsprojectparser.h"

#include "qbsproject.h"
#include "qbsprojectmanagerconstants.h"
#include "qbsprojectmanagertr.h"
#include "qbssettings.h"

#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/qtcassert.h>

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QJsonArray>

using namespace Utils;

namespace QbsProjectManager::Internal {

// --------------------------------------------------------------------
// QbsProjectParser:
// --------------------------------------------------------------------

QbsProjectParser::QbsProjectParser(QbsBuildSystem *buildSystem)
    : m_projectFilePath(buildSystem->project()->projectFilePath()),
      m_session(buildSystem->session())
{
    m_fi = new QFutureInterface<bool>();
    m_fi->setProgressRange(0, 0);
    Core::ProgressManager::addTask(m_fi->future(),
                                   Tr::tr("Reading Project \"%1\"")
                                       .arg(buildSystem->project()->displayName()),
                                   "Qbs.QbsEvaluate");
    m_fi->reportStarted();
    auto * const watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::canceled, this, &QbsProjectParser::cancel);
    watcher->setFuture(m_fi->future());
}

QbsProjectParser::~QbsProjectParser()
{
    if (m_parsing) {
        m_session->disconnect(this);
        cancel();
    }

    if (m_fi) {
        m_fi->reportCanceled();
        m_fi->reportFinished();
        delete m_fi;
    }
}

void QbsProjectParser::parse(const Store &config, const Environment &env,
                             const FilePath &dir, const QString &configName)
{
    QTC_ASSERT(m_session, return);
    QTC_ASSERT(!dir.isEmpty(), return);

    m_environment = env;
    QJsonObject request;
    request.insert("type", "resolve-project");
    Store userConfig = config;
    request.insert("top-level-profile",
                   userConfig.take(Constants::QBS_CONFIG_PROFILE_KEY).toString());
    request.insert("configuration-name", configName);
    request.insert("force-probe-execution",
                   userConfig.take(Constants::QBS_FORCE_PROBES_KEY).toBool());
    if (QbsSettings::useCreatorSettingsDirForQbs())
        request.insert("settings-directory", QbsSettings::qbsSettingsBaseDir());
    request.insert("overridden-properties", QJsonObject::fromVariantMap(mapFromStore(userConfig)));

    // People don't like it when files are created as a side effect of opening a project,
    // so do not store the build graph if the build directory does not exist yet.
    request.insert("dry-run", !dir.exists());

    request.insert("build-root", dir.path());
    request.insert("project-file-path", m_projectFilePath.path());
    request.insert("override-build-graph-data", true);
    static const auto envToJson = [](const Environment &env) {
        QJsonObject envObj;
        env.forEachEntry([&](const QString &key, const QString &value, bool enabled) {
            if (enabled)
                envObj.insert(key, value);
        });
        return envObj;
    };
    request.insert("environment", envToJson(env));
    request.insert("error-handling-mode", "relaxed");
    request.insert("data-mode", "only-if-changed");
    QbsSession::insertRequestedModuleProperties(request);

    connect(m_session, &QbsSession::projectResolved, this, [this](const ErrorInfo &error) {
        m_error = error;
        m_projectData = m_session->projectData();
        finish(!m_error.hasError());
    });
    connect(m_session, &QbsSession::errorOccurred, this, [this] { finish(false); });
    connect(m_session, &QbsSession::taskStarted, this,
            [this](const QString &description, int maxProgress) {
        Q_UNUSED(description)
        if (m_fi)
            m_fi->setProgressRange(0, maxProgress);
    });
    connect(m_session, &QbsSession::maxProgressChanged, this, [this](int maxProgress) {
        if (m_fi)
            m_fi->setProgressRange(0, maxProgress);
    });
    connect(m_session, &QbsSession::taskProgress, this, [this](int progress) {
        if (m_fi)
            m_fi->setProgressValue(progress);
    });
    m_parsing = true;
    m_session->sendRequest(request);
}

void QbsProjectParser::cancel()
{
    if (m_session)
        m_session->cancelCurrentJob();
}

void QbsProjectParser::finish(bool success)
{
    m_parsing = false;
    m_session->disconnect(this);
    if (!success)
        m_fi->reportCanceled();
    m_fi->reportFinished();
    delete m_fi;
    m_fi = nullptr;
    emit done(success);
}

} // QbsProjectManager::Internal
