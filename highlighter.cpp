#include "highlighter.h"

#include <QColor>
#include <QFileInfo>


// Small helper to build a colored text format
static QTextCharFormat makeFormat(const char *hex)
{
    QTextCharFormat format;

    format.setForeground(QColor(hex));

    return format;
}


Highlighter::Highlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
{
}


void Highlighter::addRule(const QString &pattern, const QTextCharFormat &format)
{
    HighlightingRule rule;

    rule.pattern = QRegularExpression(pattern);
    rule.format = format;

    rules.append(rule);
}


void Highlighter::setLanguage(Language language)
{
    rules.clear();

    commentStartExpression = QRegularExpression();
    commentEndExpression = QRegularExpression();

    switch (language)
    {
        case Cpp:
            setupCpp();
            break;

        case Python:
            setupPython();
            break;

        case JavaScript:
            setupJavaScript();
            break;

        case Json:
            setupJson();
            break;

        case Qml:
            setupQml();
            break;

        case CMake:
            setupCMake();
            break;

        case Html:
            setupHtml();
            break;

        default:
            break;
    }

    rehighlight();
}


void Highlighter::highlightBlock(const QString &text)
{
    // Apply all single-line rules
    for (const HighlightingRule &rule : rules)
    {
        QRegularExpressionMatchIterator iterator =
            rule.pattern.globalMatch(text);

        while (iterator.hasNext())
        {
            QRegularExpressionMatch match = iterator.next();

            setFormat(
                match.capturedStart(),
                match.capturedLength(),
                rule.format
                );
        }
    }

    // Multi-line comments (C-style blocks, HTML comments)
    if (commentStartExpression.isValid())
    {
        setCurrentBlockState(0);

        int startIndex = 0;

        if (previousBlockState() != 1)
        {
            startIndex = text.indexOf(commentStartExpression);
        }

        while (startIndex >= 0)
        {
            QRegularExpressionMatch match =
                commentEndExpression.match(text, startIndex);

            int endIndex = match.capturedStart();

            int commentLength;

            if (endIndex == -1)
            {
                // Comment continues on the next line
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            }
            else
            {
                commentLength =
                    endIndex - startIndex + match.capturedLength();
            }

            setFormat(startIndex, commentLength, multilineCommentFormat);

            startIndex = text.indexOf(
                commentStartExpression,
                startIndex + commentLength
                );
        }
    }
    else
    {
        setCurrentBlockState(0);
    }
}


Highlighter::Language Highlighter::languageForFile(const QString &fileName)
{
    QString lower = fileName.toLower();

    QString suffix = QFileInfo(lower).suffix();

    if (lower == "cmakelists.txt" || suffix == "cmake")
    {
        return CMake;
    }

    QStringList cppSuffixes = {
        "h", "hpp", "hh", "hxx", "c", "cc", "cpp", "cxx",
        "ipp", "inl", "tpp", "moc"
    };

    if (cppSuffixes.contains(suffix))
    {
        return Cpp;
    }

    if (suffix == "py" || suffix == "pyw")
    {
        return Python;
    }

    QStringList jsSuffixes = {
        "js", "mjs", "ts", "jsx", "tsx"
    };

    if (jsSuffixes.contains(suffix))
    {
        return JavaScript;
    }

    if (suffix == "json" || suffix == "jsonc")
    {
        return Json;
    }

    if (suffix == "qml")
    {
        return Qml;
    }

    QStringList htmlSuffixes = {
        "html", "htm", "xml", "svg", "ui"
    };

    if (htmlSuffixes.contains(suffix))
    {
        return Html;
    }

    return PlainText;
}


QString Highlighter::languageName(Language language)
{
    switch (language)
    {
        case Cpp:
            return "C++";

        case Python:
            return "Python";

        case JavaScript:
            return "JavaScript";

        case Json:
            return "JSON";

        case Qml:
            return "QML";

        case CMake:
            return "CMake";

        case Html:
            return "HTML/XML";

        default:
            return "Plain Text";
    }
}


void Highlighter::setupCpp()
{
    // Function names (before keywords so keywords win)
    QTextCharFormat functionFormat = makeFormat("#dcdcaa");

    addRule("\\b[A-Za-z_]\\w*(?=\\s*\\()", functionFormat);

    // Types
    QTextCharFormat typeFormat = makeFormat("#4ec9b0");

    QStringList types = {
        "size_t", "ssize_t", "int8_t", "uint8_t", "int16_t", "uint16_t",
        "int32_t", "uint32_t", "int64_t", "uint64_t",
        "qint8", "qint16", "qint32", "qint64",
        "quint8", "quint16", "quint32", "quint64",
        "QString", "QStringList", "QByteArray", "QList", "QVector",
        "QMap", "QHash", "QSet", "QPair", "std"
    };

    addRule("\\b(?:" + types.join('|') + ")\\b", typeFormat);

    // Keywords
    QTextCharFormat keywordFormat = makeFormat("#569cd6");

    QStringList keywords = {
        "alignas", "alignof", "auto", "bool", "break", "case", "catch",
        "char", "class", "const", "consteval", "constexpr", "const_cast",
        "continue", "decltype", "default", "delete", "do", "double",
        "dynamic_cast", "else", "enum", "explicit", "export", "extern",
        "false", "float", "for", "friend", "goto", "if", "inline", "int",
        "long", "mutable", "namespace", "new", "noexcept", "nullptr",
        "operator", "override", "final", "private", "protected", "public",
        "register", "reinterpret_cast", "return", "short", "signed",
        "sizeof", "static", "static_assert", "static_cast", "struct",
        "switch", "template", "this", "thread_local", "throw", "true",
        "try", "typedef", "typeid", "typename", "union", "unsigned",
        "using", "virtual", "void", "volatile", "wchar_t", "while",
        "Q_OBJECT", "Q_PROPERTY", "Q_SIGNALS", "Q_SLOTS", "signals",
        "slots", "emit", "NULL"
    };

    addRule("\\b(?:" + keywords.join('|') + ")\\b", keywordFormat);

    // Numbers
    QTextCharFormat numberFormat = makeFormat("#b5cea8");

    addRule(
        "\\b(?:0[xX][0-9a-fA-F]+|\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?)[uUlLfF]*\\b",
        numberFormat
        );

    // Preprocessor directives
    QTextCharFormat preprocessorFormat = makeFormat("#c586c0");

    addRule("^\\s*#\\s*\\w+", preprocessorFormat);

    // Strings and characters
    QTextCharFormat stringFormat = makeFormat("#ce9178");

    addRule("\"(?:\\\\.|[^\"\\\\])*\"", stringFormat);
    addRule("'(?:\\\\.|[^'\\\\])'", stringFormat);

    // Comments
    QTextCharFormat commentFormat = makeFormat("#6a9955");

    addRule("//[^\\n]*", commentFormat);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
    multilineCommentFormat = commentFormat;
}


void Highlighter::setupPython()
{
    // Function names
    QTextCharFormat functionFormat = makeFormat("#dcdcaa");

    addRule("\\b[A-Za-z_]\\w*(?=\\s*\\()", functionFormat);

    // Decorators
    QTextCharFormat decoratorFormat = makeFormat("#dcdcaa");

    addRule("^\\s*@[A-Za-z_][\\w.]*", decoratorFormat);

    // Keywords
    QTextCharFormat keywordFormat = makeFormat("#569cd6");

    QStringList keywords = {
        "and", "as", "assert", "async", "await", "break", "class",
        "continue", "def", "del", "elif", "else", "except", "finally",
        "for", "from", "global", "if", "import", "in", "is", "lambda",
        "nonlocal", "not", "or", "pass", "raise", "return", "try",
        "while", "with", "yield"
    };

    addRule("\\b(?:" + keywords.join('|') + ")\\b", keywordFormat);

    // Constants
    QTextCharFormat constantFormat = makeFormat("#569cd6");

    addRule("\\b(?:True|False|None|self|cls)\\b", constantFormat);

    // Numbers
    QTextCharFormat numberFormat = makeFormat("#b5cea8");

    addRule(
        "\\b(?:0[xX][0-9a-fA-F]+|\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?)\\b",
        numberFormat
        );

    // Strings (with optional prefixes and triple quotes)
    QTextCharFormat stringFormat = makeFormat("#ce9178");

    addRule(
        "(?:[rRbBuUfF]{0,2})(?:\"\"\"[^\\n]*\"\"\"|'''[^\\n]*'''"
        "|\"(?:\\\\.|[^\"\\\\\\n])*\"|'(?:\\\\.|[^'\\\\\\n])*')",
        stringFormat
        );

    // Comments
    QTextCharFormat commentFormat = makeFormat("#6a9955");

    addRule("#[^\\n]*", commentFormat);
}


void Highlighter::setupJavaScript()
{
    // Function names
    QTextCharFormat functionFormat = makeFormat("#dcdcaa");

    addRule("\\b[A-Za-z_]\\w*(?=\\s*\\()", functionFormat);

    // Keywords
    QTextCharFormat keywordFormat = makeFormat("#569cd6");

    QStringList keywords = {
        "async", "await", "break", "case", "catch", "class", "const",
        "continue", "debugger", "default", "delete", "do", "else",
        "export", "extends", "finally", "for", "from", "function", "get",
        "if", "import", "in", "instanceof", "let", "new", "of", "return",
        "set", "static", "super", "switch", "this", "throw", "try",
        "typeof", "var", "void", "while", "yield"
    };

    addRule("\\b(?:" + keywords.join('|') + ")\\b", keywordFormat);

    // Constants
    QTextCharFormat constantFormat = makeFormat("#569cd6");

    addRule("\\b(?:null|true|false|undefined|NaN|Infinity)\\b", constantFormat);

    // Numbers
    QTextCharFormat numberFormat = makeFormat("#b5cea8");

    addRule(
        "\\b(?:0[xX][0-9a-fA-F]+|\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?)\\b",
        numberFormat
        );

    // Strings (including template literals)
    QTextCharFormat stringFormat = makeFormat("#ce9178");

    addRule("\"(?:\\\\.|[^\"\\\\\\n])*\"", stringFormat);
    addRule("'(?:\\\\.|[^'\\\\\\n])*'", stringFormat);
    addRule("`(?:\\\\.|[^`\\\\])*`", stringFormat);

    // Comments
    QTextCharFormat commentFormat = makeFormat("#6a9955");

    addRule("//[^\\n]*", commentFormat);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
    multilineCommentFormat = commentFormat;
}


void Highlighter::setupJson()
{
    // Strings
    QTextCharFormat stringFormat = makeFormat("#ce9178");

    addRule("\"(?:\\\\.|[^\"\\\\])*\"", stringFormat);

    // Object keys (after strings so keys win)
    QTextCharFormat keyFormat = makeFormat("#9cdcfe");

    addRule("\"(?:\\\\.|[^\"\\\\])*\"(?=\\s*:)", keyFormat);

    // Numbers
    QTextCharFormat numberFormat = makeFormat("#b5cea8");

    addRule(
        "-?\\b\\d+(?:\\.\\d+)?(?:[eE][+-]?\\d+)?\\b",
        numberFormat
        );

    // Constants
    QTextCharFormat constantFormat = makeFormat("#569cd6");

    addRule("\\b(?:true|false|null)\\b", constantFormat);
}


void Highlighter::setupQml()
{
    // Reuse the JavaScript rules
    setupJavaScript();

    // QML types (capitalized identifiers)
    QTextCharFormat typeFormat = makeFormat("#4ec9b0");

    addRule("\\b[A-Z]\\w*\\b", typeFormat);

    // QML keywords
    QTextCharFormat keywordFormat = makeFormat("#569cd6");

    QStringList keywords = {
        "property", "signal", "readonly", "id", "component",
        "pragma", "import", "on"
    };

    addRule("\\b(?:" + keywords.join('|') + ")\\b", keywordFormat);
}


void Highlighter::setupCMake()
{
    // Commands
    QTextCharFormat functionFormat = makeFormat("#dcdcaa");

    QStringList commands = {
        "cmake_minimum_required", "project", "set", "unset", "option",
        "list", "message", "include", "add_executable", "add_library",
        "add_subdirectory", "add_custom_command", "add_custom_target",
        "add_compile_options", "add_compile_definitions",
        "add_link_options", "add_dependencies", "add_test",
        "find_package", "find_library", "find_path", "find_program",
        "target_link_libraries", "target_include_directories",
        "target_compile_definitions", "target_compile_options",
        "target_sources", "target_link_options",
        "set_target_properties", "set_property", "get_property",
        "get_target_property", "install", "enable_testing",
        "enable_language", "string", "file", "math",
        "execute_process", "configure_file", "foreach", "endforeach",
        "function", "endfunction", "macro", "endmacro", "if", "elseif",
        "else", "endif", "while", "endwhile", "break", "continue",
        "return", "mark_as_advanced", "include_directories",
        "link_directories", "try_compile", "try_run",
        "qt_add_executable", "qt_standard_project_setup",
        "qt_add_translations", "qt_generate_deploy_app_script"
    };

    addRule(
        "\\b(?:" + commands.join('|') + ")\\b(?=\\s*\\()",
        functionFormat
        );

    // Constants
    QTextCharFormat constantFormat = makeFormat("#569cd6");

    addRule("\\b(?:TRUE|FALSE|ON|OFF|YES|NO)\\b", constantFormat);

    // Numbers
    QTextCharFormat numberFormat = makeFormat("#b5cea8");

    addRule("\\b\\d+(?:\\.\\d+)?\\b", numberFormat);

    // Strings
    QTextCharFormat stringFormat = makeFormat("#ce9178");

    addRule("\"(?:\\\\.|[^\"\\\\])*\"", stringFormat);

    // Comments
    QTextCharFormat commentFormat = makeFormat("#6a9955");

    addRule("#[^\\n]*", commentFormat);
}


void Highlighter::setupHtml()
{
    // Tags
    QTextCharFormat tagFormat = makeFormat("#569cd6");

    addRule("</?\\s*[A-Za-z][A-Za-z0-9:-]*", tagFormat);
    addRule("/?>", tagFormat);

    // Attributes
    QTextCharFormat attributeFormat = makeFormat("#9cdcfe");

    addRule("[A-Za-z_][\\w:.-]*(?=\\s*=)", attributeFormat);

    // Strings
    QTextCharFormat stringFormat = makeFormat("#ce9178");

    addRule("\"[^\"]*\"", stringFormat);

    // Comments
    QTextCharFormat commentFormat = makeFormat("#6a9955");

    commentStartExpression = QRegularExpression("<!--");
    commentEndExpression = QRegularExpression("-->");
    multilineCommentFormat = commentFormat;
}