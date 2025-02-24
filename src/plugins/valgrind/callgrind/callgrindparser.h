// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "callgrindparsedata.h"

#include <QObject>

namespace Utils { class FilePath; }

namespace Valgrind::Callgrind {

/**
 * Parser for Valgrind --tool=callgrind output
 * most of the format is documented at http://kcachegrind.sourceforge.net/html/CallgrindFormat.html
 *
 * FIXME: most length asserts are not correct, see documentation 1.2:
 * "If a cost line specifies less event counts than given in the "events" line,
 * the rest is assumed to be zero."
 *
 */

ParseDataPtr parseDataFile(const Utils::FilePath &filePath);

} // namespace Valgrind::Callgrind
