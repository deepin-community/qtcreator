// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "dapengine.h"

namespace Debugger::Internal {

class PyDapEngine : public DapEngine
{
public:
    PyDapEngine();

private:
    void handleDapInitialize() override;
    void quitDebugger() override;

    void setupEngine() override;
    bool isLocalAttachEngine() const;

    bool acceptsBreakpoint(const BreakpointParameters &bp) const override;

    const QLoggingCategory &logCategory() override;

    std::unique_ptr<Utils::Process> m_installProcess;
};

} // Debugger::Internal
