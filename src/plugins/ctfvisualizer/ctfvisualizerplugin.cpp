// Copyright (C) 2019 Klarälvdalens Datakonsult AB, a KDAB Group company,
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "ctfvisualizertool.h"

#include <extensionsystem/iplugin.h>

namespace CtfVisualizer::Internal {

class CtfVisualizerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "CtfVisualizer.json")

    void initialize() final
    {
        setupCtfVisualizerTool(this);
    }
};

} // CtfVisualizer::Internal

#include "ctfvisualizerplugin.moc"
