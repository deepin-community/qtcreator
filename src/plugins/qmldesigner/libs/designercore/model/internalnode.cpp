// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "internalbindingproperty.h"
#include "internalnode_p.h"
#include "internalnodeabstractproperty.h"
#include "internalnodelistproperty.h"
#include "internalnodeproperty.h"
#include "internalproperty.h"
#include "internalsignalhandlerproperty.h"
#include "internalvariantproperty.h"

#include <designeralgorithm.h>
#include <utils/algorithm.h>

#include <QDebug>

#include <algorithm>
#include <ranges>
#include <utility>

namespace QmlDesigner {
namespace Internal {

void InternalNode::setParentProperty(const InternalNodeAbstractProperty::Pointer &parent)
{
    InternalNodeAbstractProperty::Pointer parentProperty = m_parentProperty.lock();
    if (parentProperty)
        parentProperty->remove(shared_from_this());

    Q_ASSERT(parent && parent->isValid());
    m_parentProperty = parent;

    parent->add(shared_from_this());
}

void InternalNode::resetParentProperty()
{
    InternalNodeAbstractProperty::Pointer parentProperty = m_parentProperty.lock();
    if (parentProperty)
        parentProperty->remove(shared_from_this());

    m_parentProperty.reset();
}

std::optional<QVariant> InternalNode::auxiliaryData(AuxiliaryDataKeyView key) const
{
    auto found = std::ranges::find(m_auxiliaryDatas, key, &AuxiliaryData::first);

    if (found != m_auxiliaryDatas.end())
        return found->second;

    return {};
}

bool InternalNode::setAuxiliaryData(AuxiliaryDataKeyView key, const QVariant &data)
{
    auto found = std::ranges::find(m_auxiliaryDatas, key, &AuxiliaryData::first);

    if (found != m_auxiliaryDatas.end()) {
        if (found->second == data)
            return false;
        found->second = data;
    } else {
        m_auxiliaryDatas.emplace_back(AuxiliaryDataKey{key}, data);
    }

    return true;
}

bool InternalNode::removeAuxiliaryData(AuxiliaryDataKeyView key)
{
    auto found = std::ranges::find(m_auxiliaryDatas, key, &AuxiliaryData::first);

    if (found == m_auxiliaryDatas.end())
        return false;

    *found = std::move(m_auxiliaryDatas.back());

    m_auxiliaryDatas.pop_back();

    return true;
}

bool InternalNode::hasAuxiliaryData(AuxiliaryDataKeyView key) const
{
    auto found = std::ranges::find(m_auxiliaryDatas, key, &AuxiliaryData::first);

    return found != m_auxiliaryDatas.end();
}

bool InternalNode::hasAuxiliaryData(AuxiliaryDataType type) const
{
    auto found = std::ranges::find(m_auxiliaryDatas, type, [](const auto &e) { return e.first.type; });

    return found != m_auxiliaryDatas.end();
}

AuxiliaryDatasForType InternalNode::auxiliaryData(AuxiliaryDataType type) const
{
    AuxiliaryDatasForType data;
    data.reserve(m_auxiliaryDatas.size());

    for (const auto &element : m_auxiliaryDatas) {
        if (element.first.type == type)
            data.emplace_back(element.first.name, element.second);
    }

    return data;
}

PropertyNameList InternalNode::propertyNameList() const
{
    return CoreUtils::to<PropertyNameList>(m_nameProperties | std::views::keys
                                           | std::views::transform(&Utils::SmallString::toQByteArray));
}

PropertyNameViews InternalNode::propertyNameViews() const
{
    return CoreUtils::to<PropertyNameViews>(m_nameProperties | std::views::keys);
}

InternalNode::ManyNodes InternalNode::allSubNodes() const
{
    ManyNodes nodes;
    nodes.reserve(1024);

    addSubNodes(nodes);

    return nodes;
}

void InternalNode::addSubNodes(ManyNodes &nodes, const InternalProperty *property)
{
    switch (property->type()) {
    case PropertyType::NodeList:
        property->to<PropertyType::NodeList>()->addSubNodes(nodes);
        break;
    case PropertyType::Node:
        property->to<PropertyType::Node>()->addSubNodes(nodes);
        break;
    case PropertyType::Binding:
    case PropertyType::None:
    case PropertyType::SignalDeclaration:
    case PropertyType::SignalHandler:
    case PropertyType::Variant:
        break;
    }
}

void InternalNode::addSubNodes(ManyNodes &nodes) const
{
    for (const auto &entry : m_nameProperties)
        addSubNodes(nodes, entry.second.get());
}

void InternalNode::addDirectSubNodes(ManyNodes &nodes) const
{
    for (const auto &entry : m_nameProperties)
        addDirectSubNodes(nodes, entry.second.get());
}

namespace {
void append(InternalNode::ManyNodes &nodes, auto &appendNodes)
{
    std::copy(appendNodes.begin(), appendNodes.end(), std::back_inserter(nodes));
}
} // namespace

void InternalNode::addDirectSubNodes(ManyNodes &nodes, const InternalProperty *property)
{
    switch (property->type()) {
    case PropertyType::NodeList:
        append(nodes, property->to<PropertyType::NodeList>()->nodes());
        break;
    case PropertyType::Node:
        nodes.append(property->to<PropertyType::Node>()->node());
        break;
    case PropertyType::Binding:
    case PropertyType::None:
    case PropertyType::SignalDeclaration:
    case PropertyType::SignalHandler:
    case PropertyType::Variant:
        break;
    }
}

InternalNode::ManyNodes InternalNode::allDirectSubNodes() const
{
    ManyNodes nodes;
    nodes.reserve(96);

    addDirectSubNodes(nodes);

    return nodes;
}

} // namespace Internal
} // namespace QmlDesigner
