// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "androidmanager.h"
#include "androidsdkmanager.h"
#include "androidsdkmodel.h"
#include "androidtr.h"

#include <utils/algorithm.h>
#include <utils/qtcassert.h>
#include <utils/utilsicons.h>

#include <QIcon>

namespace Android {
namespace Internal {

const int packageColCount = 3;

AndroidSdkModel::AndroidSdkModel(AndroidSdkManager *sdkManager, QObject *parent)
    : QAbstractItemModel(parent),
      m_sdkManager(sdkManager)
{
    QTC_CHECK(m_sdkManager);
    connect(m_sdkManager, &AndroidSdkManager::packagesReloaded, this, &AndroidSdkModel::refreshData);
    refreshData();
}

QVariant AndroidSdkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    QVariant data;
    if (role == Qt::DisplayRole) {
        switch (section) {
        case packageNameColumn:
            data = Tr::tr("Package");
            break;
        case packageRevisionColumn:
            data = Tr::tr("Revision");
            break;
        case apiLevelColumn:
            data = Tr::tr("API");
            break;
        default:
            break;
        }
    }
    return data;
}

QModelIndex AndroidSdkModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        // Packages under top items.
        if (parent.row() == 0) {
            // Tools packages
            if (row < m_tools.count())
                return createIndex(row, column, const_cast<AndroidSdkPackage *>(m_tools.at(row)));
        } else if (parent.row() < m_sdkPlatforms.count() + 1) {
            // Platform packages
            const SdkPlatform *sdkPlatform = m_sdkPlatforms.at(parent.row() - 1);
            SystemImageList images = sdkPlatform->systemImages(AndroidSdkPackage::AnyValidState);
            if (row < images.count() + 1) {
                if (row == 0)
                    return createIndex(row, column, const_cast<SdkPlatform *>(sdkPlatform));
                else
                    return createIndex(row, column, images.at(row - 1));
            }
        }
    } else if (row < m_sdkPlatforms.count() + 1) {
        return createIndex(row, column); // Top level items (Tools & platform)
    }

    return {};
}

QModelIndex AndroidSdkModel::parent(const QModelIndex &index) const
{
    void *ip = index.internalPointer();
    if (!ip)
        return {};

    auto package = static_cast<const AndroidSdkPackage *>(ip);
    if (package->type() == AndroidSdkPackage::SystemImagePackage) {
        auto image = static_cast<const SystemImage *>(package);
        int row = m_sdkPlatforms.indexOf(const_cast<SdkPlatform *>(image->platform()));
        if (row > -1)
            return createIndex(row + 1, 0);
    } else if (package->type() == AndroidSdkPackage::SdkPlatformPackage) {
        int row = m_sdkPlatforms.indexOf(static_cast<const SdkPlatform *>(package));
        if (row > -1)
            return createIndex(row + 1, 0);
    } else {
        return createIndex(0, 0); // Tools
    }

    return {};
}

int AndroidSdkModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_sdkPlatforms.count() + 1;

    if (!parent.internalPointer()) {
        if (parent.row() == 0) // Tools
            return m_tools.count();

        if (parent.row() <= m_sdkPlatforms.count()) {
            const SdkPlatform * platform = m_sdkPlatforms.at(parent.row() - 1);
            return platform->systemImages(AndroidSdkPackage::AnyValidState).count() + 1;
        }
    }

    return 0;
}

int AndroidSdkModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return packageColCount;
}

QVariant AndroidSdkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (!index.parent().isValid()) {
        // Top level tools
        if (index.row() == 0) {
            return role == Qt::DisplayRole && index.column() == packageNameColumn ?
                        QVariant(Tr::tr("Tools")) : QVariant();
        }
        // Top level platforms
        const SdkPlatform *platform = m_sdkPlatforms.at(index.row() - 1);
        if (role == Qt::DisplayRole) {
            if (index.column() == packageNameColumn) {
                const QString androidName = AndroidManager::androidNameForApiLevel(
                                                platform->apiLevel())
                                            + platform->extension();
                if (androidName.startsWith("Android"))
                    return androidName;
                else
                    return platform->displayText();
            } else if (index.column() == apiLevelColumn) {
                return platform->apiLevel();
            }
        }
        return {};
    }

    auto p = static_cast<const AndroidSdkPackage *>(index.internalPointer());
    QString apiLevelStr;
    if (p->type() == AndroidSdkPackage::SdkPlatformPackage)
        apiLevelStr = QString::number(static_cast<const SdkPlatform *>(p)->apiLevel());

    if (p->type() == AndroidSdkPackage::SystemImagePackage)
        apiLevelStr = QString::number(static_cast<const SystemImage *>(p)->platform()->apiLevel());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case packageNameColumn:
            return p->type() == AndroidSdkPackage::SdkPlatformPackage ?
                        Tr::tr("SDK Platform") : p->displayText();
        case packageRevisionColumn:
            return p->revision().toString();
        case apiLevelColumn:
            return apiLevelStr;
        default:
            break;
        }
    }

    if (index.column() == packageNameColumn) {
        if (role == Qt::CheckStateRole) {
            if (p->state() == AndroidSdkPackage::Installed)
                return m_changeState.contains(p) ? Qt::Unchecked : Qt::Checked;
            else
                return m_changeState.contains(p) ? Qt::Checked : Qt::Unchecked;
        }

        if (role == Qt::FontRole) {
            QFont font;
            if (m_changeState.contains(p))
                font.setBold(true);
            return font;
        }
    }

    if (role == Qt::TextAlignmentRole && index.column() == packageRevisionColumn)
        return Qt::AlignRight;

    if (role == Qt::ToolTipRole)
        return QString("%1 - (%2)").arg(p->descriptionText()).arg(p->sdkStylePath());

    if (role == PackageTypeRole)
        return p->type();

    if (role == PackageStateRole)
        return p->state();

    return {};
}

QHash<int, QByteArray> AndroidSdkModel::roleNames() const
{
    QHash <int, QByteArray> roles;
    roles[PackageTypeRole] = "PackageRole";
    roles[PackageStateRole] = "PackageState";
    return roles;
}

Qt::ItemFlags AndroidSdkModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if (index.column() == packageNameColumn)
        f |= Qt::ItemIsUserCheckable;

    void *ip = index.internalPointer();
    if (ip && index.column() == packageNameColumn) {
        auto package = static_cast<const AndroidSdkPackage *>(ip);
        if (package->state() == AndroidSdkPackage::Installed
                && package->type() == AndroidSdkPackage::SdkToolsPackage) {
            f &= ~Qt::ItemIsEnabled;
        }
    }
    return f;
}

bool AndroidSdkModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    void *ip = index.internalPointer();
    if (ip && role == Qt::CheckStateRole) {
        auto package = static_cast<const AndroidSdkPackage *>(ip);
        if (value.toInt() == Qt::Checked && package->state() != AndroidSdkPackage::Installed) {
            m_changeState << package;
            emit dataChanged(index, index, {Qt::CheckStateRole});
        } else if (m_changeState.remove(package)) {
            emit dataChanged(index, index, {Qt::CheckStateRole});
        } else if (value.toInt() == Qt::Unchecked) {
            m_changeState.insert(package);
            emit dataChanged(index, index, {Qt::CheckStateRole});
        }
        return true;
    }
    return false;
}

InstallationChange AndroidSdkModel::installationChange() const
{
    if (m_changeState.isEmpty())
        return {};

    InstallationChange change;
    for (const AndroidSdkPackage *package : m_changeState) {
        if (package->state() == AndroidSdkPackage::Installed)
            change.toUninstall << package->sdkStylePath();
        else
            change.toInstall << package->sdkStylePath();
    }
    return change;
}

void AndroidSdkModel::resetSelection()
{
    beginResetModel();
    m_changeState.clear();
    endResetModel();
}

void AndroidSdkModel::refreshData()
{
    m_sdkPlatforms.clear();
    m_tools.clear();
    m_changeState.clear();
    beginResetModel();
    for (AndroidSdkPackage *p : m_sdkManager->allSdkPackages()) {
        if (p->type() == AndroidSdkPackage::SdkPlatformPackage)
            m_sdkPlatforms << static_cast<SdkPlatform *>(p);
        else
            m_tools << p;
    }
    Utils::sort(m_sdkPlatforms, [](const SdkPlatform *p1, const SdkPlatform *p2) {
        return p1->apiLevel() > p2->apiLevel();
    });

    Utils::sort(m_tools, [](const AndroidSdkPackage *p1, const AndroidSdkPackage *p2) {
        if (p1->state() == p2->state())
            return p1->type() == p2->type() ? p1->revision() > p2->revision() : p1->type() > p2->type();
        else
            return p1->state() < p2->state();
    });
    endResetModel();
}

} // namespace Internal
} // namespace Android
