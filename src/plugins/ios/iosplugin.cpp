// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "iosbuildconfiguration.h"
#include "iosbuildstep.h"
#include "iosconfigurations.h"
#include "iosconstants.h"
#include "iosdeploystep.h"
#include "iosdevice.h"
#include "iosdsymbuildstep.h"
#include "iosqtversion.h"
#include "iosrunner.h"
#include "iossettingspage.h"
#include "iossimulator.h"
#include "iostoolhandler.h"
#include "iostr.h"
#include "iosrunconfiguration.h"

#include <extensionsystem/iplugin.h>

#include <projectexplorer/deployconfiguration.h>
#include <projectexplorer/devicesupport/devicemanager.h>
#include <projectexplorer/runconfiguration.h>

using namespace ProjectExplorer;

namespace Ios::Internal {

Q_LOGGING_CATEGORY(iosLog, "qtc.ios.common", QtWarningMsg)

class IosDeployConfigurationFactory : public DeployConfigurationFactory
{
public:
    IosDeployConfigurationFactory()
    {
        setConfigBaseId("Qt4ProjectManager.IosDeployConfiguration");
        addSupportedTargetDeviceType(Constants::IOS_DEVICE_TYPE);
        addSupportedTargetDeviceType(Constants::IOS_SIMULATOR_TYPE);
        setDefaultDisplayName(Tr::tr("Deploy on iOS"));
        addInitialStep(Constants::IOS_DEPLOY_STEP_ID);
    }
};

class IosPluginPrivate
{
public:
    IosSimulatorFactory simulatorFactory;
    IosBuildStepFactory buildStepFactory;
    IosDeployStepFactory deployStepFactory;
    IosDsymBuildStepFactory dsymBuildStepFactory;
    IosDeployConfigurationFactory deployConfigurationFactory;
    IosRunWorkerFactory runWorkerFactory;
    IosDebugWorkerFactory debugWorkerFactory;
    IosQmlProfilerWorkerFactory qmlProfilerWorkerFactory;
};

class IosPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Ios.json")

    ~IosPlugin() final
    {
        delete d;
    }

    void initialize() final
    {
        qRegisterMetaType<Ios::IosToolHandler::Dict>("Ios::IosToolHandler::Dict");

        setupIosToolchain();
        setupIosBuildConfiguration();
        setupIosQtVersion();
        setupIosDevice();

        IosConfigurations::initialize();

        setupIosRunConfiguration();
        setupIosSettingsPage();

        d = new IosPluginPrivate;
    }

    IosPluginPrivate *d = nullptr;
};

} // Internal::Ios

#include "iosplugin.moc"
