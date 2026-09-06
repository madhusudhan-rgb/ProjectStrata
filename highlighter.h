#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QStringList>

class QTextDocument;

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    enum Language
    {
        PlainText,
        Cpp,
        Python,
        JavaScript,
        Json,
        Qml,
        CMake,
        Html
    };

    explicit Highlighter(QTextDocument *document = nullptr);

    void setLanguage(Language language);

    static Language languageForFile(const QString &fileName);
    static QString languageName(Language language);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void setupCpp();
    void setupPython();
    void setupJavaScript();
    void setupJson();
    void setupQml();
    void setupCMake();
    void setupHtml();

    void addRule(const QString &pattern, const QTextCharFormat &format);

    QList<HighlightingRule> rules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QTextCharFormat multilineCommentFormat;
};