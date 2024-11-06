// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qttestparser.h"

#include "qtoutputformatter.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>

#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

using namespace ProjectExplorer;
using namespace Utils;

namespace QtSupport::Internal {

OutputLineParser::Result QtTestParser::handleLine(const QString &line, OutputFormat type)
{
    if (type != StdOutFormat && type != DebugFormat)
        return Status::NotHandled;

    const QString theLine = rightTrimmed(line);
    static const QRegularExpression triggerPattern("^(?:XPASS|FAIL!)  : .+$");
    QTC_CHECK(triggerPattern.isValid());
    if (triggerPattern.match(theLine).hasMatch()) {
        emitCurrentTask();
        m_currentTask = Task(Task::Error, theLine, FilePath(), -1,
                             Constants::TASK_CATEGORY_AUTOTEST);
        return {Status::InProgress, {}, {}, StdErrFormat};
    }
    if (m_currentTask.isNull())
        return Status::NotHandled;
    static const QRegularExpression locationPattern(HostOsInfo::isWindowsHost()
        ? QString(QT_TEST_FAIL_WIN_REGEXP)
        : QString(QT_TEST_FAIL_UNIX_REGEXP));
    QTC_CHECK(locationPattern.isValid());
    const QRegularExpressionMatch match = locationPattern.match(theLine);
    if (match.hasMatch()) {
        LinkSpecs linkSpecs;
        m_currentTask.file = absoluteFilePath(FilePath::fromString(
                    QDir::fromNativeSeparators(match.captured("file"))));
        m_currentTask.line = match.captured("line").toInt();
        addLinkSpecForAbsoluteFilePath(
            linkSpecs, m_currentTask.file, m_currentTask.line, m_currentTask.column, match, "file");
        emitCurrentTask();
        return {Status::Done, linkSpecs};
    }
    if (line.startsWith("   Actual") || line.startsWith("   Expected")) {
        m_currentTask.details.append(theLine);
        return Status::InProgress;
    }
    return Status::NotHandled;
}

void QtTestParser::emitCurrentTask()
{
    if (!m_currentTask.isNull()) {
        scheduleTask(m_currentTask, 1);
        m_currentTask.clear();
    }
}

} // QtSupport::Internal


#ifdef WITH_TESTS

#include <projectexplorer/outputparser_test.h>

#include <QTest>

namespace QtSupport::Internal {

class QtTestParserTest final : public QObject
{
    Q_OBJECT

public slots:
    void testQtTestOutputParser();
};

void QtTestParserTest::testQtTestOutputParser()
{
    OutputParserTester testbench;
    testbench.addLineParser(new QtTestParser);
    const QString input = "random output\n"
            "PASS   : MyTest::someTest()\n"
            "XPASS  : MyTest::someTest()\n"
#ifdef Q_OS_WIN
            "C:\\dev\\tests\\tst_mytest.cpp(154) : failure location\n"
#else
            "   Loc: [/home/me/tests/tst_mytest.cpp(154)]\n"
#endif
            "FAIL!  : MyTest::someOtherTest(init) Compared values are not the same\n"
            "   Actual   (exceptionCaught): 0\n"
            "   Expected (true)           : 1\n"
#ifdef Q_OS_WIN
            "C:\\dev\\tests\\tst_mytest.cpp(220) : failure location\n"
#else
            "   Loc: [/home/me/tests/tst_mytest.cpp(220)]\n"
#endif
            "XPASS: irrelevant\n"
            "PASS   : MyTest::anotherTest()";
    const QString expectedChildOutput =
            "random output\n"
            "PASS   : MyTest::someTest()\n"
            "XPASS: irrelevant\n"
            "PASS   : MyTest::anotherTest()\n";
    const FilePath theFile = FilePath::fromString(HostOsInfo::isWindowsHost()
        ? QString("C:/dev/tests/tst_mytest.cpp") : QString("/home/me/tests/tst_mytest.cpp"));
    const Tasks expectedTasks{
        Task(Task::Error, "XPASS  : MyTest::someTest()", theFile, 154,
             Constants::TASK_CATEGORY_AUTOTEST),
        Task(Task::Error, "FAIL!  : MyTest::someOtherTest(init) "
                          "Compared values are not the same\n"
                          "   Actual   (exceptionCaught): 0\n"
                          "   Expected (true)           : 1",
             theFile, 220, Constants::TASK_CATEGORY_AUTOTEST)};
    testbench.testParsing(input, OutputParserTester::STDOUT, expectedTasks, expectedChildOutput,
                          QString(), QString());
}

QObject *createQtTestParserTest()
{
    return new QtTestParserTest;
}

} // QtSupport::Internal

#include "qttestparser.moc"

#endif // WITH_TESTS
