# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

source("../../shared/qtcreator.py")

def verifyProjectsMode(expectedKits):
    treeView = waitForObject(":Projects.ProjectNavigationTreeView")
    bAndRIndex = getQModelIndexStr("text='Build & Run'",
                                   ":Projects.ProjectNavigationTreeView")
    foundKits = dumpItems(treeView.model(), waitForObject(bAndRIndex))
    # ignore Python kits and non-kit item
    excludes = ('Python', 'Hide Inactive Kits', 'Show All Kits')
    relevantKits = list(filter(lambda x: all(ex not in x for ex in excludes), foundKits))
    test.compare(len(relevantKits), len(expectedKits), "Verify number of listed kits.")
    test.compare(set(relevantKits), set(expectedKits), "Verify if expected kits are listed.")
    hasKits = len(expectedKits) > 0
    test.verify(checkIfObjectExists(":scrollArea.Edit build configuration:_QLabel", hasKits),
                "Verify if build settings are being displayed.")
    test.verify(checkIfObjectExists(":No valid kits found._QLabel", not hasKits),
                "Verify if Creator reports missing kits.")

kitNameTemplate = "Manual.%s"


def __removeKit__(_, kitName):
    global kitNameTemplate
    if 'Python' in kitName: # ignore Python kits
        return
    item = kitNameTemplate % kitName.replace(".", "\\.")
    if kitName == Targets.getStringForTarget(Targets.getDefaultKit()):
        item += " (default)"
    mouseClick(waitForObjectItem(":BuildAndRun_QTreeView", item))
    clickButton(waitForObject(":Remove_QPushButton"))

def main():
    startQC()
    if not startedWithoutPluginError():
        return
    createProject_Qt_Console(tempDir(), "SquishProject", buildSystem = "qmake")
    switchViewTo(ViewConstants.PROJECTS)
    verifyProjectsMode(Targets.getTargetsAsStrings(Targets.availableTargetClasses(True)))
    iterateKits(True, False, __removeKit__)
    verifyProjectsMode([])
    invokeMenuItem("File", "Exit")
