// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once

#include <QWidget>

namespace Android::Internal {

class AndroidSdkManager;

void executeAndroidSdkManagerDialog(AndroidSdkManager *sdkManager, QWidget *parent);

} // Android::Internal
