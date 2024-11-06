// Copyright (C) 2016 BogDan Vatra <bog_dan_ro@yahoo.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <qtsupport/baseqtversion.h>

#include <projectexplorer/gcctoolchain.h>

namespace Android::Internal {

using ToolchainList = QList<ProjectExplorer::Toolchain *>;

class AndroidToolchain : public ProjectExplorer::GccToolchain
{
public:
    explicit AndroidToolchain();
    ~AndroidToolchain() override;

    bool isValid() const override;
    void addToEnvironment(Utils::Environment &env) const override;

    QStringList suggestedMkspecList() const override;
    Utils::FilePath makeCommand(const Utils::Environment &environment) const override;
    void fromMap(const Utils::Store &data) override;

    void setNdkLocation(const Utils::FilePath &ndkLocation);
    Utils::FilePath ndkLocation() const;

protected:
    DetectedAbisResult detectSupportedAbis() const override;

private:
    mutable Utils::FilePath m_ndkLocation;
};

ToolchainList autodetectToolchains(const ToolchainList &alreadyKnown);
ToolchainList autodetectToolchainsFromNdks(const ToolchainList &alreadyKnown,
                                           const QList<Utils::FilePath> &ndkLocations,
                                           const bool isCustom = false);

void setupAndroidToolchain();

} // Android
