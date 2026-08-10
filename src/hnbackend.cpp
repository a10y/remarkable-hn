#include "hnbackend.h"

#include "readability.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {

constexpr auto apiBase = "https://hacker-news.firebaseio.com/v0/";
constexpr int storyLimit = 10;
constexpr int commentLimit = 80;
constexpr int commentDepthLimit = 5;

QUrl itemUrl(qint64 id)
{
    return QUrl(QStringLiteral("%1item/%2.json").arg(QString::fromLatin1(apiBase)).arg(id));
}

QVariantMap jsonMap(QNetworkReply *reply)
{
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    return document.isObject() ? document.object().toVariantMap() : QVariantMap{};
}

} // namespace

HnBackend::HnBackend(QObject *parent)
    : QObject(parent)
{
}

QNetworkReply *HnBackend::get(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("HN Reader for reMarkable/0.1"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(20000);
    return m_network.get(request);
}

void HnBackend::refreshStories()
{
    if (m_loadingStories)
        return;

    setError({});
    m_stories.clear();
    emit storiesChanged();
    setLoadingStories(true);

    QNetworkReply *reply = get(QUrl(QStringLiteral("%1topstories.json").arg(
        QString::fromLatin1(apiBase))));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            setError(QStringLiteral("Could not load Hacker News: %1").arg(reply->errorString()));
            setLoadingStories(false);
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray ids = document.array();
        const int count = qMin(storyLimit, ids.size());
        if (count == 0) {
            setError(QStringLiteral("Hacker News returned no stories."));
            setLoadingStories(false);
            return;
        }

        m_storySlots = QVector<QVariantMap>(count);
        m_pendingStories = count;
        for (int index = 0; index < count; ++index)
            fetchStory(ids.at(index).toInteger(), index);
    });
}

void HnBackend::fetchStory(qint64 id, int index)
{
    QNetworkReply *reply = get(itemUrl(id));
    connect(reply, &QNetworkReply::finished, this, [this, reply, index] {
        const auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });
        if (reply->error() == QNetworkReply::NoError)
            m_storySlots[index] = storyMap(jsonMap(reply));

        --m_pendingStories;
        if (m_pendingStories != 0)
            return;

        m_stories.clear();
        for (const QVariantMap &story : std::as_const(m_storySlots)) {
            if (!story.isEmpty())
                m_stories.append(story);
        }
        emit storiesChanged();
        if (m_stories.isEmpty())
            setError(QStringLiteral("The top stories could not be loaded."));
        setLoadingStories(false);
    });
}

QVariantMap HnBackend::storyMap(const QVariantMap &json)
{
    QVariantMap story;
    story.insert(QStringLiteral("id"), json.value(QStringLiteral("id")));
    story.insert(QStringLiteral("title"), json.value(QStringLiteral("title")));
    story.insert(QStringLiteral("url"), json.value(QStringLiteral("url")));
    story.insert(QStringLiteral("text"), json.value(QStringLiteral("text")));
    story.insert(QStringLiteral("by"), json.value(QStringLiteral("by")));
    story.insert(QStringLiteral("score"), json.value(QStringLiteral("score")));
    story.insert(QStringLiteral("comments"), json.value(QStringLiteral("descendants"), 0));
    story.insert(QStringLiteral("host"), hostForUrl(json.value(QStringLiteral("url")).toString()));
    story.insert(QStringLiteral("age"), relativeTime(json.value(QStringLiteral("time")).toLongLong()));
    return story;
}

void HnBackend::openStory(int index)
{
    if (index < 0 || index >= m_stories.size())
        return;

    m_selectedStory = m_stories.at(index).toMap();
    m_articleHtml.clear();
    m_comments.clear();
    emit selectedStoryChanged();
    emit articleChanged();
    emit commentsChanged();
    emit storyOpened();
    fetchArticle();
}

void HnBackend::retryArticle()
{
    if (!m_selectedStory.isEmpty())
        fetchArticle();
}

void HnBackend::fetchArticle()
{
    if (m_loadingArticle)
        return;

    setError({});
    const QString url = m_selectedStory.value(QStringLiteral("url")).toString();
    if (url.isEmpty()) {
        const QString text = m_selectedStory.value(QStringLiteral("text")).toString();
        m_articleHtml = text.isEmpty()
            ? QStringLiteral("<p>This Hacker News post has no linked article or body text.</p>")
            : Readability::commentFromHtml(text);
        emit articleChanged();
        return;
    }

    setLoadingArticle(true);
    QNetworkReply *reply = get(QUrl(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            setError(QStringLiteral("Could not load the article: %1").arg(reply->errorString()));
            setLoadingArticle(false);
            return;
        }

        m_articleHtml = Readability::articleFromHtml(QString::fromUtf8(reply->readAll()));
        emit articleChanged();
        setLoadingArticle(false);
    });
}

void HnBackend::loadComments()
{
    if (m_loadingComments || m_selectedStory.isEmpty())
        return;

    setError({});
    m_comments.clear();
    m_commentQueue.clear();
    m_commentCount = 0;
    emit commentsChanged();
    setLoadingComments(true);

    QNetworkReply *reply = get(itemUrl(m_selectedStory.value(QStringLiteral("id")).toLongLong()));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            setError(QStringLiteral("Could not load comments: %1").arg(reply->errorString()));
            setLoadingComments(false);
            return;
        }

        const QVariantList kids = jsonMap(reply).value(QStringLiteral("kids")).toList();
        for (const QVariant &kid : kids)
            m_commentQueue.enqueue({kid.toLongLong(), 0});
        fetchNextComment();
    });
}

void HnBackend::fetchNextComment()
{
    if (m_commentQueue.isEmpty() || m_commentCount >= commentLimit) {
        emit commentsChanged();
        setLoadingComments(false);
        return;
    }

    const CommentRequest request = m_commentQueue.dequeue();
    QNetworkReply *reply = get(itemUrl(request.id));
    connect(reply, &QNetworkReply::finished, this, [this, reply, request] {
        const auto deleteReply = qScopeGuard([reply] { reply->deleteLater(); });
        if (reply->error() == QNetworkReply::NoError) {
            const QVariantMap json = jsonMap(reply);
            const bool deleted = json.value(QStringLiteral("deleted")).toBool() ||
                                 json.value(QStringLiteral("dead")).toBool();
            if (!deleted) {
                QVariantMap comment;
                comment.insert(QStringLiteral("by"), json.value(QStringLiteral("by"),
                                                                 QStringLiteral("anonymous")));
                comment.insert(QStringLiteral("age"),
                               relativeTime(json.value(QStringLiteral("time")).toLongLong()));
                comment.insert(QStringLiteral("text"),
                               Readability::commentFromHtml(json.value(QStringLiteral("text")).toString()));
                comment.insert(QStringLiteral("depth"), request.depth);
                m_comments.append(comment);
                ++m_commentCount;
                if (m_commentCount % 10 == 0)
                    emit commentsChanged();
            }

            if (request.depth < commentDepthLimit) {
                const QVariantList kids = json.value(QStringLiteral("kids")).toList();
                for (auto it = kids.crbegin(); it != kids.crend(); ++it)
                    m_commentQueue.prepend({it->toLongLong(), request.depth + 1});
            }
        }
        fetchNextComment();
    });
}

QString HnBackend::relativeTime(qint64 timestamp)
{
    const qint64 seconds = qMax<qint64>(0, QDateTime::currentSecsSinceEpoch() - timestamp);
    if (seconds < 60)
        return QStringLiteral("now");
    if (seconds < 3600)
        return QStringLiteral("%1m").arg(seconds / 60);
    if (seconds < 86400)
        return QStringLiteral("%1h").arg(seconds / 3600);
    return QStringLiteral("%1d").arg(seconds / 86400);
}

QString HnBackend::hostForUrl(const QString &url)
{
    QString host = QUrl(url).host();
    if (host.startsWith(QStringLiteral("www.")))
        host.remove(0, 4);
    return host.isEmpty() ? QStringLiteral("news.ycombinator.com") : host;
}

void HnBackend::setError(const QString &message)
{
    if (m_errorMessage == message)
        return;
    m_errorMessage = message;
    emit errorMessageChanged();
}

void HnBackend::setLoadingStories(bool loading)
{
    if (m_loadingStories == loading)
        return;
    m_loadingStories = loading;
    emit loadingStoriesChanged();
}

void HnBackend::setLoadingArticle(bool loading)
{
    if (m_loadingArticle == loading)
        return;
    m_loadingArticle = loading;
    emit loadingArticleChanged();
}

void HnBackend::setLoadingComments(bool loading)
{
    if (m_loadingComments == loading)
        return;
    m_loadingComments = loading;
    emit loadingCommentsChanged();
}

void HnBackend::quit()
{
    QCoreApplication::quit();
}
