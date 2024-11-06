// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "sdkmanageroutputparser.h"

#include "androidconstants.h"
#include "androidsdkpackage.h"
#include "avdmanageroutputparser.h"

#include <utils/algorithm.h>

#include <QRegularExpression>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(sdkManagerLog, "qtc.android.sdkManager", QtWarningMsg)
const char installLocationKey[] = "Installed Location:";
const char revisionKey[] = "Version:";
const char descriptionKey[] = "Description:";
} // namespace

namespace Android::Internal {

using MarkerTagsType = std::map<SdkManagerOutputParser::MarkerTag, const char *>;
Q_GLOBAL_STATIC_WITH_ARGS(MarkerTagsType, markerTags,
                          ({{SdkManagerOutputParser::MarkerTag::InstalledPackagesMarker, "Installed packages:"},
                            {SdkManagerOutputParser::MarkerTag::AvailablePackagesMarkers, "Available Packages:"},
                            {SdkManagerOutputParser::MarkerTag::AvailableUpdatesMarker, "Available Updates:"},
                            {SdkManagerOutputParser::MarkerTag::PlatformMarker, "platforms"},
                            {SdkManagerOutputParser::MarkerTag::SystemImageMarker, "system-images"},
                            {SdkManagerOutputParser::MarkerTag::BuildToolsMarker, "build-tools"},
                            {SdkManagerOutputParser::MarkerTag::SdkToolsMarker, "tools"},
                            {SdkManagerOutputParser::MarkerTag::CmdlineSdkToolsMarker, Constants::cmdlineToolsName},
                            {SdkManagerOutputParser::MarkerTag::PlatformToolsMarker, "platform-tools"},
                            {SdkManagerOutputParser::MarkerTag::EmulatorToolsMarker, "emulator"},
                            {SdkManagerOutputParser::MarkerTag::NdkMarker, Constants::ndkPackageName},
                            {SdkManagerOutputParser::MarkerTag::ExtrasMarker, "extras"}}));

class GenericPackageData
{
public:
    bool isValid() const { return !revision.isNull() && !description.isNull(); }
    QStringList headerParts;
    QVersionNumber revision;
    QString description;
    Utils::FilePath installedLocation;
    QMap<QString, QString> extraData;
};

void SdkManagerOutputParser::parsePackageListing(const QString &output)
{
    QStringList packageData;
    bool collectingPackageData = false;
    MarkerTag currentPackageMarker = MarkerTag::None;

    auto processCurrentPackage = [&] {
        if (collectingPackageData) {
            collectingPackageData = false;
            parsePackageData(currentPackageMarker, packageData);
            packageData.clear();
        }
    };

    static const QRegularExpression delimiters("[\\n\\r]");
    const auto lines = output.split(delimiters);
    for (const QString &outputLine : lines) {

        // NOTE: we don't want to parse Dependencies part as it does not add value
        if (outputLine.startsWith("        "))
            continue;

        // We don't need to parse this because they would still be listed on available packages
        if (m_currentSection == AvailableUpdatesMarker)
            continue;

        MarkerTag marker = parseMarkers(outputLine.trimmed());
        if (marker & SectionMarkers) {
            // Section marker found. Update the current section being parsed.
            m_currentSection = marker;
            processCurrentPackage();
            continue;
        }

        if (m_currentSection == None)
            continue; // Continue with the verbose output until a valid section starts.

        if (marker == EmptyMarker) {
            // Empty marker. Occurs at the end of a package details.
            // Process the collected package data, if any.
            processCurrentPackage();
            continue;
        }

        if (marker == None) {
            if (collectingPackageData)
                packageData << outputLine; // Collect data until next marker.
            else
                continue;
        } else {
            // Package marker found.
            processCurrentPackage(); // New package starts. Process the collected package data, if any.
            currentPackageMarker = marker;
            collectingPackageData = true;
            packageData << outputLine;
        }
    }
    compilePackageAssociations();
}

void SdkManagerOutputParser::compilePackageAssociations()
{
    // Return true if package p is already installed i.e. there exists a installed package having
    // same sdk style path and same revision as of p.
    auto isInstalled = [](const AndroidSdkPackageList &container, AndroidSdkPackage *p) {
        return Utils::anyOf(container, [p](AndroidSdkPackage *other) {
            return other->state() == AndroidSdkPackage::Installed &&
                   other->sdkStylePath() == p->sdkStylePath() &&
                   other->revision() == p->revision();
        });
    };

    auto deleteAlreadyInstalled = [isInstalled](AndroidSdkPackageList &packages) {
        for (auto p = packages.begin(); p != packages.end();) {
            if ((*p)->state() == AndroidSdkPackage::Available && isInstalled(packages, *p)) {
                delete *p;
                p = packages.erase(p);
            } else {
                ++p;
            }
        }
    };

    // Remove already installed packages.
    deleteAlreadyInstalled(m_packages);

    // Filter out available images that are already installed.
    AndroidSdkPackageList images = m_systemImages.keys();
    deleteAlreadyInstalled(images);

    // Associate the system images with sdk platforms.
    for (AndroidSdkPackage *image : std::as_const(images)) {
        int imageApi = m_systemImages[image];
        auto itr = std::find_if(m_packages.begin(), m_packages.end(),
                                [imageApi](const AndroidSdkPackage *p) {
                                    const SdkPlatform *platform = nullptr;
                                    if (p->type() == AndroidSdkPackage::SdkPlatformPackage)
                                        platform = static_cast<const SdkPlatform*>(p);
                                    return platform && platform->apiLevel() == imageApi;
                                });
        if (itr != m_packages.end()) {
            auto platform = static_cast<SdkPlatform*>(*itr);
            platform->addSystemImage(static_cast<SystemImage *>(image));
        }
    }
}

void SdkManagerOutputParser::parsePackageData(MarkerTag packageMarker, const QStringList &data)
{
    QTC_ASSERT(!data.isEmpty() && packageMarker != None, return);

    AndroidSdkPackage *package = nullptr;
    auto createPackage = [&](std::function<AndroidSdkPackage *(SdkManagerOutputParser *,
                                                               const QStringList &)> creator) {
        if ((package = creator(this, data)))
            m_packages.append(package);
    };

    switch (packageMarker) {
    case MarkerTag::BuildToolsMarker:
        createPackage(&SdkManagerOutputParser::parseBuildToolsPackage);
        break;

    case MarkerTag::SdkToolsMarker:
        createPackage(&SdkManagerOutputParser::parseSdkToolsPackage);
        break;

    case MarkerTag::CmdlineSdkToolsMarker:
        createPackage(&SdkManagerOutputParser::parseSdkToolsPackage);
        break;

    case MarkerTag::PlatformToolsMarker:
        createPackage(&SdkManagerOutputParser::parsePlatformToolsPackage);
        break;

    case MarkerTag::EmulatorToolsMarker:
        createPackage(&SdkManagerOutputParser::parseEmulatorToolsPackage);
        break;

    case MarkerTag::PlatformMarker:
        createPackage(&SdkManagerOutputParser::parsePlatform);
        break;

    case MarkerTag::SystemImageMarker:
    {
        QPair<SystemImage *, int> result = parseSystemImage(data);
        if (result.first) {
            m_systemImages[result.first] = result.second;
            package = result.first;
        }
    }
    break;

    case MarkerTag::NdkMarker:
        createPackage(&SdkManagerOutputParser::parseNdkPackage);
        break;

    case MarkerTag::ExtrasMarker:
        createPackage(&SdkManagerOutputParser::parseExtraToolsPackage);
        break;

    case MarkerTag::GenericToolMarker:
        createPackage(&SdkManagerOutputParser::parseGenericTools);
        break;

    default:
        qCDebug(sdkManagerLog) << "Unhandled package: " << markerTags->at(packageMarker);
        break;
    }

    if (package) {
        switch (m_currentSection) {
        case MarkerTag::InstalledPackagesMarker:
            package->setState(AndroidSdkPackage::Installed);
            break;
        case MarkerTag::AvailablePackagesMarkers:
        case MarkerTag::AvailableUpdatesMarker:
            package->setState(AndroidSdkPackage::Available);
            break;
        default:
            qCDebug(sdkManagerLog) << "Invalid section marker: " << markerTags->at(m_currentSection);
            break;
        }
    }
}

/*!
    Parses the \a line for a [spaces]key[spaces]value[spaces] pattern and returns
    \c true if \a key is found, false otherwise. Result is copied into \a value.
 */
static bool valueForKey(QString key, const QString &line, QString *value = nullptr)
{
    auto trimmedInput = line.trimmed();
    if (trimmedInput.startsWith(key)) {
        if (value)
            *value = trimmedInput.section(key, 1, 1).trimmed();
        return true;
    }
    return false;
}

static bool parseAbstractData(GenericPackageData &output, const QStringList &input, int minParts,
                              const QString &logStrTag, const QStringList &extraKeys = {})
{
    if (input.isEmpty()) {
        qCDebug(sdkManagerLog) << logStrTag + ": Empty input";
        return false;
    }

    output.headerParts = input.at(0).split(';');
    if (output.headerParts.count() < minParts) {
        qCDebug(sdkManagerLog) << logStrTag + "%1: Unexpected header:" << input;
        return false;
    }

    QStringList keys = extraKeys;
    keys << installLocationKey << revisionKey << descriptionKey;
    for (const QString &line : input) {
        QString value;
        for (const auto &key: std::as_const(keys)) {
            if (valueForKey(key, line, &value)) {
                if (key == installLocationKey)
                    output.installedLocation = Utils::FilePath::fromUserInput(value);
                else if (key == revisionKey)
                    output.revision = QVersionNumber::fromString(value);
                else if (key == descriptionKey)
                    output.description = value;
                else
                    output.extraData[key] = value;
                break;
            }
        }
    }

    return output.isValid();
}

template <typename T>
AndroidSdkPackage *parsePackage(const QStringList &data, int minParts, const QString &logStrTag)
{
    AndroidSdkPackage *package = nullptr;
    GenericPackageData packageData;
    if (parseAbstractData(packageData, data, minParts, logStrTag)) {
        if constexpr (std::is_same_v<SdkPlatform, T>) {
            const int apiLevel = platformNameToApiLevel(packageData.headerParts.at(1));
            if (apiLevel == -1) {
                qCDebug(sdkManagerLog) << "Platform: Cannot parse api level:"<< data;
                return nullptr;
            }
            package = new T(packageData.revision, data.at(0), apiLevel);
            package->setExtension(convertNameToExtension(packageData.headerParts.at(1)));
        } else {
            package = new T(packageData.revision, data.at(0));
        }
        package->setDescriptionText(packageData.description);
        package->setDisplayText(packageData.description);
        package->setInstalledLocation(packageData.installedLocation);
    } else {
        qCDebug(sdkManagerLog) << logStrTag + ':'
                               << "Parsing failed. Minimum required data unavailable:" << data;
    }
    return package;
}

AndroidSdkPackage *SdkManagerOutputParser::parsePlatform(const QStringList &data) const
{
    return parsePackage<SdkPlatform>(data, 2, "Platform");
}

QPair<SystemImage *, int> SdkManagerOutputParser::parseSystemImage(const QStringList &data) const
{
    QPair <SystemImage *, int> result(nullptr, -1);
    GenericPackageData packageData;
    if (parseAbstractData(packageData, data, 4, "System-image")) {
        const int apiLevel = platformNameToApiLevel(packageData.headerParts.at(1));
        if (apiLevel == -1) {
            qCDebug(sdkManagerLog) << "System-image: Cannot parse api level:"<< data;
            return result;
        }
        auto image = new SystemImage(packageData.revision, data.at(0),
                                     packageData.headerParts.at(3));
        image->setInstalledLocation(packageData.installedLocation);
        image->setDisplayText(packageData.description);
        image->setDescriptionText(packageData.description);
        image->setApiLevel(apiLevel);
        result = {image, apiLevel};
    } else {
        qCDebug(sdkManagerLog) << "System-image: Minimum required data unavailable: "<< data;
    }
    return result;
}

AndroidSdkPackage *SdkManagerOutputParser::parseBuildToolsPackage(const QStringList &data) const
{
    return parsePackage<BuildTools>(data, 2, "Build-tools");
}

AndroidSdkPackage *SdkManagerOutputParser::parseSdkToolsPackage(const QStringList &data) const
{
    return parsePackage<SdkTools>(data, 1, "SDK-tools");
}

AndroidSdkPackage *SdkManagerOutputParser::parsePlatformToolsPackage(const QStringList &data) const
{
    return parsePackage<PlatformTools>(data, 1, "Platform-tools");
}

AndroidSdkPackage *SdkManagerOutputParser::parseEmulatorToolsPackage(const QStringList &data) const
{
    return parsePackage<EmulatorTools>(data, 1, "Emulator-tools");
}

AndroidSdkPackage *SdkManagerOutputParser::parseNdkPackage(const QStringList &data) const
{
    return parsePackage<Ndk>(data, 1, "NDK");
}

AndroidSdkPackage *SdkManagerOutputParser::parseExtraToolsPackage(const QStringList &data) const
{
    return parsePackage<ExtraTools>(data, 1, "Extra-tools");
}

AndroidSdkPackage *SdkManagerOutputParser::parseGenericTools(const QStringList &data) const
{
    return parsePackage<GenericSdkPackage>(data, 1, "Generic");
}

SdkManagerOutputParser::MarkerTag SdkManagerOutputParser::parseMarkers(const QString &line)
{
    if (line.isEmpty())
        return EmptyMarker;

    for (auto pair : *markerTags) {
        if (line.startsWith(QLatin1String(pair.second)))
            return pair.first;
    }
    static const QRegularExpression reg("^[a-zA-Z]+[A-Za-z0-9;._-]+");
    const QRegularExpressionMatch match = reg.match(line);
    if (match.hasMatch() && match.captured(0) == line)
        return GenericToolMarker;

    return None;
}

} // namespace Android::Internal
