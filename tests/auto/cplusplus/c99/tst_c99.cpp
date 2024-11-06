// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <cplusplus/CPlusPlus.h>
#include <cplusplus/CppDocument.h>
#include <cplusplus/LookupContext.h>

#include <QtTest>
#include <QObject>
#include <QFile>

//TESTED_COMPONENT=src/libs/cplusplus
using namespace CPlusPlus;

#define VERIFY_ERRORS() \
    do { \
      QByteArray expectedErrors; \
      if (!errorFile.isEmpty()) { \
        QFile e(testdata(errorFile)); \
        if (e.open(QFile::ReadOnly)) \
          expectedErrors = QTextStream(&e).readAll().toUtf8(); \
      } \
      QCOMPARE(QString::fromLatin1(errors), QString::fromLatin1(expectedErrors)); \
    } while (0)


class tst_c99: public QObject
{
    Q_OBJECT

    /*
        Returns the path to some testdata file or directory.
    */
    static QString testdata(const QString &name = QString())
    {
        static const QString dataDirectory = QLatin1String(SRCDIR "/data");

        QString result = dataDirectory;
        if (!name.isEmpty()) {
            result += QLatin1Char('/');
            result += name;
        }
        return result;
    }

    struct Client: DiagnosticClient {
        QByteArray *errors;

        Client(QByteArray *errors)
            : errors(errors)
        {
        }

        void report(int level,
                    const StringLiteral *fileName,
                    int line, int column,
                    const char *format, va_list ap) override
        {
            if (! errors)
                return;

            static const char *const pretty[] = {"warning", "error", "fatal"};

            QString str = QString::asprintf("%s:%d:%d: %s: ", fileName->chars(), line, column, pretty[level]);
            errors->append(str.toUtf8());

            str += QString::vasprintf(format, ap);
            errors->append(str.toUtf8());

            errors->append('\n');
        }
    };

    Document::Ptr document(const QString &fileName, QByteArray *errors = 0)
    {
        Document::Ptr doc = Document::create(Utils::FilePath::fromString(fileName));
        QFile file(testdata(fileName));
        if (file.open(QFile::ReadOnly)) {
            LanguageFeatures features;
            features.c99Enabled = true;
            Client client(errors);
            doc->control()->setDiagnosticClient(&client);
            doc->setUtf8Source(QTextStream(&file).readAll().toUtf8());
            doc->translationUnit()->setLanguageFeatures(features);
            doc->check();
            doc->control()->setDiagnosticClient(0);
        } else {
            qWarning() << "could not read file" << fileName;
        }
        return doc;
    }

private Q_SLOTS:
    //
    // checks for the syntax
    //
    void parse_data();
    void parse();
};


void tst_c99::parse_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("errorFile");

    QTest::newRow("designatedInitializer.1") << "designatedInitializer.1.c" << "";
    QTest::newRow("designatedInitializer.2") << "designatedInitializer.2.c" << "";
    QTest::newRow("limits-caselabels (QTCREATORBUG-12673)") << "limits-caselabels.c" << "";
}

void tst_c99::parse()
{
    QFETCH(QString, file);
    QFETCH(QString, errorFile);

    QByteArray errors;
    Document::Ptr doc = document(file, &errors);

    if (! qgetenv("DEBUG").isNull())
        printf("%s\n", errors.constData());

    VERIFY_ERRORS();
}

QTEST_APPLESS_MAIN(tst_c99)
#include "tst_c99.moc"
