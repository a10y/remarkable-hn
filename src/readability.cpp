#include "readability.h"

#include <QRegularExpression>
#include <QStringList>

namespace {

QString removeBlocks(QString html, const QString &tagNames)
{
    const QRegularExpression block(
        QStringLiteral("<(?:%1)\\b[^>]*>.*?</(?:%1)\\s*>").arg(tagNames),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    QString previous;
    do {
        previous = html;
        html.remove(block);
    } while (html != previous);
    return html;
}

QString extractTag(const QString &html, const QString &tag)
{
    const QRegularExpression expression(
        QStringLiteral("<%1\\b[^>]*>(.*)</%1\\s*>").arg(tag),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption |
            QRegularExpression::InvertedGreedinessOption);
    QString best;
    auto matches = expression.globalMatch(html);
    while (matches.hasNext()) {
        const QString candidate = matches.next().captured(1);
        if (Readability::plainText(candidate).size() > Readability::plainText(best).size())
            best = candidate;
    }
    return best;
}

QString sanitize(QString html, bool article)
{
    html.remove(QRegularExpression(
        QStringLiteral("<!--.*?-->"),
        QRegularExpression::DotMatchesEverythingOption));
    html = removeBlocks(
        html,
        article ? QStringLiteral("script|style|noscript|svg|canvas|form|nav|header|footer|aside|button|figure|picture")
                : QStringLiteral("script|style|noscript|svg|canvas|form|button"));

    html.replace(QRegularExpression(QStringLiteral("<(?:div|section|article|main)\\b[^>]*>"),
                                    QRegularExpression::CaseInsensitiveOption),
                 QStringLiteral("<p>"));
    html.replace(QRegularExpression(QStringLiteral("</(?:div|section|article|main)\\s*>"),
                                    QRegularExpression::CaseInsensitiveOption),
                 QStringLiteral("</p>"));
    html.replace(QRegularExpression(QStringLiteral("<hr\\b[^>]*>"),
                                    QRegularExpression::CaseInsensitiveOption),
                 QStringLiteral("<p>--------------------</p>"));

    const QRegularExpression allowedOpening(
        QStringLiteral("<(p|br|h1|h2|h3|h4|blockquote|pre|code|ul|ol|li|em|strong|b|i|a)\\b[^>]*>"),
        QRegularExpression::CaseInsensitiveOption);
    html.replace(allowedOpening, QStringLiteral("<\\1>"));
    html.remove(QRegularExpression(
        QStringLiteral("<(?!/?(?:p|br|h1|h2|h3|h4|blockquote|pre|code|ul|ol|li|em|strong|b|i|a)\\b)[^>]+>"),
        QRegularExpression::CaseInsensitiveOption));
    html.replace(QRegularExpression(QStringLiteral("<(?:a)>(.*?)</a>"),
                                    QRegularExpression::CaseInsensitiveOption |
                                        QRegularExpression::DotMatchesEverythingOption),
                 QStringLiteral("\\1"));
    html.replace(QRegularExpression(QStringLiteral("(?:<p>\\s*</p>\\s*)+"),
                                    QRegularExpression::CaseInsensitiveOption),
                 QStringLiteral(""));
    html.replace(QRegularExpression(QStringLiteral("[ \\t]+")), QStringLiteral(" "));
    html.replace(QRegularExpression(QStringLiteral("\\n{3,}")), QStringLiteral("\n\n"));
    return html.trimmed();
}

} // namespace

namespace Readability {

QString plainText(const QString &source)
{
    QString html = source;
    html.remove(QRegularExpression(QStringLiteral("<[^>]+>"),
                                   QRegularExpression::DotMatchesEverythingOption));
    html.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "), Qt::CaseInsensitive);
    html.replace(QStringLiteral("&amp;"), QStringLiteral("&"), Qt::CaseInsensitive);
    html.replace(QStringLiteral("&lt;"), QStringLiteral("<"), Qt::CaseInsensitive);
    html.replace(QStringLiteral("&gt;"), QStringLiteral(">"), Qt::CaseInsensitive);
    html.replace(QStringLiteral("&quot;"), QStringLiteral("\""), Qt::CaseInsensitive);
    html.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    return html.trimmed();
}

QString articleFromHtml(const QString &source)
{
    QString html = source;
    html.remove(QRegularExpression(
        QStringLiteral("<!--.*?-->"),
        QRegularExpression::DotMatchesEverythingOption));
    html = removeBlocks(html, QStringLiteral("script|style|noscript|svg|canvas|form|nav|header|footer|aside"));

    QString article = extractTag(html, QStringLiteral("article"));
    const QString main = extractTag(html, QStringLiteral("main"));
    if (plainText(main).size() > plainText(article).size())
        article = main;

    if (plainText(article).size() < 300) {
        const QString body = extractTag(html, QStringLiteral("body"));
        if (!body.isEmpty())
            article = body;
    }

    article = sanitize(article, true);
    if (plainText(article).size() < 80)
        return QStringLiteral("<p>This page did not contain enough readable text.</p>");
    return article;
}

QString commentFromHtml(const QString &html)
{
    return sanitize(html, false);
}

} // namespace Readability
