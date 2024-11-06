// Copyright (C) Filippo Cucchetto <filippocucchetto@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "nimindenter.h"

#include "../tools/nimlexer.h"

#include <texteditor/icodestylepreferences.h>
#include <texteditor/tabsettings.h>
#include <texteditor/simplecodestylepreferences.h>
#include <texteditor/tabsettings.h>

#include <QSet>
#include <QDebug>

namespace Nim {

class NimIndenter final : public TextEditor::TextIndenter
{
public:
    explicit NimIndenter(QTextDocument *doc)
        : TextEditor::TextIndenter(doc)
    {}

    bool isElectricCharacter(const QChar &ch) const final
    {
        return ch == QLatin1Char(':') || ch == QLatin1Char('=');
    }

    void indentBlock(const QTextBlock &block,
                     const QChar &typedChar,
                     const TextEditor::TabSettings &settings,
                     int cursorPositionInEditor = -1) final;

private:
    bool startsBlock(const QString &line, int state) const;
    bool endsBlock(const QString &line, int state) const;

    int calculateIndentationDiff(const QString &previousLine,
                                 int previousState,
                                 int indentSize) const;
};

static QString rightTrimmed(const QString &str)
{
    int n = str.size() - 1;
    for (; n >= 0; --n) {
        if (!str.at(n).isSpace())
            return str.left(n + 1);
    }
    return QString();
}

void NimIndenter::indentBlock(const QTextBlock &block,
                              const QChar &typedChar,
                              const TextEditor::TabSettings &settings,
                              int cursorPositionInEditor)
{
    Q_UNUSED(typedChar)
    Q_UNUSED(cursorPositionInEditor)

    const QString currentLine = block.text();

    const QTextBlock previousBlock = block.previous();
    const QString previousLine = previousBlock.text();
    const int previousState = previousBlock.userState();

    if (!previousBlock.isValid()) {
        settings.indentLine(block, 0);
        return;
    }

    // Calculate indentation
    int indentation = 0;
    if (rightTrimmed(currentLine).isEmpty()) {
        // Current line is empty so we calculate indentation based on previous line
        const int indentationDiff = calculateIndentationDiff(previousLine, previousState, settings.m_indentSize);
        indentation = settings.indentationColumn(previousLine) + indentationDiff;
    }
    else {
        // We don't change indentation if the line is already indented.
        // This is safer but sub optimal
        indentation = settings.indentationColumn(block.text());
    }

    // Sets indentation
    settings.indentLine(block, std::max(0, indentation));
}

bool NimIndenter::startsBlock(const QString &line, int state) const
{
    NimLexer lexer(line.constData(), line.length(), static_cast<NimLexer::State>(state));

    // Read until end of line and save the last token
    NimLexer::Token previous;
    NimLexer::Token current = lexer.next();
    while (current.type != NimLexer::TokenType::EndOfText) {
        switch (current.type) {
        case NimLexer::TokenType::Comment:
        case NimLexer::TokenType::Documentation:
            break;
        default:
            previous = current;
            break;
        }
        current = lexer.next();
    }

    // electric characters start a new block, and are operators
    if (previous.type == NimLexer::TokenType::Operator) {
        QStringView ref = QStringView(line).mid(previous.begin, previous.length);
        return ref.isEmpty() ? false : isElectricCharacter(ref.at(0));
    }

    // some keywords starts a new block
    if (previous.type == NimLexer::TokenType::Keyword) {
        QStringView ref = QStringView(line).mid(previous.begin, previous.length);
        return ref == QLatin1String("type")
               || ref == QLatin1String("var")
               || ref == QLatin1String("let")
               || ref == QLatin1String("enum")
               || ref == QLatin1String("object");
    }

    return false;
}

bool NimIndenter::endsBlock(const QString &line, int state) const
{
    NimLexer lexer(line.constData(), line.length(), static_cast<NimLexer::State>(state));

    // Read until end of line and save the last tokens
    NimLexer::Token previous;
    NimLexer::Token current = lexer.next();
    while (current.type != NimLexer::TokenType::EndOfText) {
        previous = current;
        current = lexer.next();
    }

    // Some keywords end a block
    if (previous.type == NimLexer::TokenType::Keyword) {
        QStringView ref = QStringView(line).mid(previous.begin, previous.length);
        return ref == QLatin1String("return")
               || ref == QLatin1String("break")
               || ref == QLatin1String("continue");
    }

    return false;
}

int NimIndenter::calculateIndentationDiff(const QString &previousLine, int previousState, int indentSize) const
{
    if (previousLine.isEmpty())
        return 0;

    if (startsBlock(previousLine, previousState))
        return indentSize;

    if (endsBlock(previousLine, previousState))
        return -indentSize;

    return 0;
}

TextEditor::Indenter *createNimIndenter(QTextDocument *doc)
{
    return new NimIndenter(doc);
}

} // Nim
