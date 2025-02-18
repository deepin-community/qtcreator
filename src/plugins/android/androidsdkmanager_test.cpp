// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "androidsdkmanager_test.h"
#include "androidsdkmanager.h"

#include <QTest>

namespace Android::Internal {

class AndroidSdkManagerTest final : public QObject
{
    Q_OBJECT

private slots:
    void testAndroidSdkManagerProgressParser_data();
    void testAndroidSdkManagerProgressParser();
};

void AndroidSdkManagerTest::testAndroidSdkManagerProgressParser_data()
{
    QTest::addColumn<QString>("output");
    QTest::addColumn<int>("progress");
    QTest::addColumn<bool>("foundAssertion");

    // Output of "sdkmanager --licenses", Android SDK Tools version 4.0
    QTest::newRow("Loading local repository")
        << "Loading local repository...                                                     \r"
        << -1
        << false;

    QTest::newRow("Fetch progress (single line)")
        << "[=============                          ] 34% Fetch remote repository...        \r"
        << 34
        << false;

    QTest::newRow("Fetch progress (multi line)")
        << "[=============================          ] 73% Fetch remote repository...        \r"
           "[=============================          ] 75% Fetch remote repository...        \r"
        << 75
        << false;

    QTest::newRow("Some SDK package licenses not accepted")
        << "7 of 7 SDK package licenses not accepted.\n"
        << -1
        << false;

    QTest::newRow("Unaccepted licenses assertion")
        << "\nReview licenses that have not been accepted (y/N)? "
        << -1
        << true;
}

static int parseProgress(const QString &out, bool &foundAssertion)
{
    int progress = -1;
    if (out.isEmpty())
        return progress;
    static const QRegularExpression reg("(?<progress>\\d*)%");
    static const QRegularExpression regEndOfLine("[\\n\\r]");
    const QStringList lines = out.split(regEndOfLine, Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QRegularExpressionMatch match = reg.match(line);
        if (match.hasMatch()) {
            progress = match.captured("progress").toInt();
            if (progress < 0 || progress > 100)
                progress = -1;
        }
        if (!foundAssertion)
            foundAssertion = assertionRegExp().match(line).hasMatch();
    }
    return progress;
}

void AndroidSdkManagerTest::testAndroidSdkManagerProgressParser()
{
    QFETCH(QString, output);
    QFETCH(int, progress);
    QFETCH(bool, foundAssertion);

    bool actualFoundAssertion = false;
    const int actualProgress = parseProgress(output, actualFoundAssertion);

    QCOMPARE(progress, actualProgress);
    QCOMPARE(foundAssertion, actualFoundAssertion);
}

QObject *createAndroidSdkManagerTest()
{
    return new AndroidSdkManagerTest;
}

} // Android::Internal

#include "androidsdkmanager_test.moc"
