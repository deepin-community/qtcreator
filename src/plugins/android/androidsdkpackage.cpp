// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "androidsdkpackage.h"

#include <utils/algorithm.h>

namespace Android {

AndroidSdkPackage::AndroidSdkPackage(const QVersionNumber &version, const QString &sdkStylePathStr,
                                     QObject *parent) :
    QObject(parent),
    m_revision(version),
    m_sdkStylePath(sdkStylePathStr)
{

}

bool AndroidSdkPackage::operator <(const AndroidSdkPackage &other) const
{
    if (typeid(*this) != typeid(other))
        return type() < other.type();
    return displayText() < other.displayText();
}

QString AndroidSdkPackage::displayText() const
{
    return m_displayText;
}

QString AndroidSdkPackage::descriptionText() const
{
    return m_descriptionText;
}

const QVersionNumber &AndroidSdkPackage::revision() const
{
    return m_revision;
}

AndroidSdkPackage::PackageState AndroidSdkPackage::state() const
{
    return m_state;
}

const QString &AndroidSdkPackage::sdkStylePath() const
{
    return m_sdkStylePath;
}

const Utils::FilePath &AndroidSdkPackage::installedLocation() const
{
    return m_installedLocation;
}

void AndroidSdkPackage::setDisplayText(const QString &str)
{
    m_displayText = str;
}

void AndroidSdkPackage::setDescriptionText(const QString &str)
{
    m_descriptionText = str;
}

void AndroidSdkPackage::setState(AndroidSdkPackage::PackageState state)
{
    m_state = state;
}

void AndroidSdkPackage::setInstalledLocation(const Utils::FilePath &path)
{
    m_installedLocation = path;
    if (m_installedLocation.exists())
        updatePackageDetails();
}

void AndroidSdkPackage::setExtension(const QString &extension)
{
    m_extension = extension;
}

QString AndroidSdkPackage::extension() const
{
    return m_extension;
}

SystemImage::SystemImage(const QVersionNumber &version, const QString &sdkStylePathStr,
                         const QString &abi, SdkPlatform *platform)
    : AndroidSdkPackage(version, sdkStylePathStr, platform)
    , m_platform(platform)
    , m_abiName(abi)
{}

bool SystemImage::isValid() const
{
    return m_platform && m_platform->isValid();
}

const QString &SystemImage::abiName() const
{
    return m_abiName;
}

const SdkPlatform *SystemImage::platform() const
{
    return m_platform.data();
}

void SystemImage::setPlatform(SdkPlatform *platform)
{
    m_platform = platform;
}

int SystemImage::apiLevel() const
{
    return m_apiLevel;
}

void SystemImage::setApiLevel(const int apiLevel)
{
    m_apiLevel = apiLevel;
}

SdkPlatform::SdkPlatform(const QVersionNumber &version, const QString &sdkStylePathStr, int api)
    : AndroidSdkPackage(version, sdkStylePathStr)
    , m_apiLevel(api)
{
    setDisplayText(QString("android-%1")
                   .arg(m_apiLevel != -1 ? QString::number(m_apiLevel) : "Unknown"));
}

SdkPlatform::~SdkPlatform()
{
    for (SystemImage *image : std::as_const(m_systemImages))
        delete image;
    m_systemImages.clear();
}

bool SdkPlatform::operator <(const AndroidSdkPackage &other) const
{
    if (typeid(*this) != typeid(other))
        return AndroidSdkPackage::operator <(other);

    const auto &platform = static_cast<const SdkPlatform &>(other);
    if (platform.m_apiLevel == m_apiLevel)
        return AndroidSdkPackage::operator <(other);

    return platform.m_apiLevel < m_apiLevel;
}

int SdkPlatform::apiLevel() const
{
    return m_apiLevel;
}

QVersionNumber SdkPlatform::version() const
{
    return m_version;
}

void SdkPlatform::addSystemImage(SystemImage *image)
{
    // Ordered insert. Installed images on top with lexical comparison of the display name.
    auto itr = m_systemImages.cbegin();
    while (itr != m_systemImages.cend()) {
        SystemImage *currentImage = *itr;
        if (currentImage->state() == image->state()) {
            if (currentImage->displayText() > image->displayText())
                break;
        } else if (currentImage->state() > image->state()) {
            break;
        }
        ++itr;
    }
    m_systemImages.insert(itr, image);
    image->setPlatform(this);
}

SystemImageList SdkPlatform::systemImages(PackageState state) const
{
    return Utils::filtered(m_systemImages, [state](const SystemImage *image) {
        return image->state() & state;
    });
}

} // namespace Android
