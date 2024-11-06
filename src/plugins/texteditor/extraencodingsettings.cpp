// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "extraencodingsettings.h"

#include "texteditortr.h"
#include "texteditorsettings.h"

#include <coreplugin/icore.h>

// Keep this for compatibility reasons.
static const char kUtf8BomBehaviorKey[] = "Utf8BomBehavior";
static const char kLineEndingBehaviorKey[] = "LineEndingBehavior";

using namespace Utils;

namespace TextEditor {

ExtraEncodingSettings::ExtraEncodingSettings() : m_utf8BomSetting(OnlyKeep), m_lineEndingSetting{Unix}
{}

ExtraEncodingSettings::~ExtraEncodingSettings() = default;

Store ExtraEncodingSettings::toMap() const
{
    return {
        {kUtf8BomBehaviorKey, m_utf8BomSetting},
        {kLineEndingBehaviorKey, m_lineEndingSetting}
    };
}

void ExtraEncodingSettings::fromMap(const Store &map)
{
    m_utf8BomSetting = (Utf8BomSetting)map.value(kUtf8BomBehaviorKey, m_utf8BomSetting).toInt();
    m_lineEndingSetting = (LineEndingSetting)map.value(kLineEndingBehaviorKey, m_lineEndingSetting).toInt();
}

bool ExtraEncodingSettings::equals(const ExtraEncodingSettings &s) const
{
    return m_utf8BomSetting == s.m_utf8BomSetting && m_lineEndingSetting == s.m_lineEndingSetting;
}

QStringList ExtraEncodingSettings::lineTerminationModeNames()
{
    return {Tr::tr("Unix (LF)"), Tr::tr("Windows (CRLF)")};
}

ExtraEncodingSettings &globalExtraEncodingSettings()
{
    static ExtraEncodingSettings theGlobalExtraEncodingSettings;
    return theGlobalExtraEncodingSettings;
}

const char extraEncodingGroup[] = "textEditorManager";

void updateGlobalExtraEncodingSettings(const ExtraEncodingSettings &newExtraEncodingSettings)
{
    if (newExtraEncodingSettings.equals(globalExtraEncodingSettings()))
        return;

    globalExtraEncodingSettings() = newExtraEncodingSettings;
    storeToSettings(extraEncodingGroup, Core::ICore::settings(), globalExtraEncodingSettings().toMap());

    emit TextEditorSettings::instance()->extraEncodingSettingsChanged(newExtraEncodingSettings);
}

void setupExtraEncodingSettings()
{
    globalExtraEncodingSettings().fromMap(storeFromSettings(extraEncodingGroup, Core::ICore::settings()));
}

} // TextEditor
