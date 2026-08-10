#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>
#include <QVariantList>
#include <QVariantMap>

class QNetworkReply;

class HnBackend final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList stories READ stories NOTIFY storiesChanged)
    Q_PROPERTY(QVariantList comments READ comments NOTIFY commentsChanged)
    Q_PROPERTY(QVariantMap selectedStory READ selectedStory NOTIFY selectedStoryChanged)
    Q_PROPERTY(QString articleHtml READ articleHtml NOTIFY articleChanged)
    Q_PROPERTY(bool loadingStories READ loadingStories NOTIFY loadingStoriesChanged)
    Q_PROPERTY(bool loadingArticle READ loadingArticle NOTIFY loadingArticleChanged)
    Q_PROPERTY(bool loadingComments READ loadingComments NOTIFY loadingCommentsChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit HnBackend(QObject *parent = nullptr);

    QVariantList stories() const { return m_stories; }
    QVariantList comments() const { return m_comments; }
    QVariantMap selectedStory() const { return m_selectedStory; }
    QString articleHtml() const { return m_articleHtml; }
    bool loadingStories() const { return m_loadingStories; }
    bool loadingArticle() const { return m_loadingArticle; }
    bool loadingComments() const { return m_loadingComments; }
    QString errorMessage() const { return m_errorMessage; }

    Q_INVOKABLE void refreshStories();
    Q_INVOKABLE void openStory(int index);
    Q_INVOKABLE void retryArticle();
    Q_INVOKABLE void loadComments();
    Q_INVOKABLE void quit();

signals:
    void storiesChanged();
    void commentsChanged();
    void selectedStoryChanged();
    void articleChanged();
    void loadingStoriesChanged();
    void loadingArticleChanged();
    void loadingCommentsChanged();
    void errorMessageChanged();
    void storyOpened();

private:
    struct CommentRequest {
        qint64 id;
        int depth;
    };

    QNetworkReply *get(const QUrl &url);
    void fetchStory(qint64 id, int index);
    void fetchArticle();
    void fetchNextComment();
    void setError(const QString &message);
    void setLoadingStories(bool loading);
    void setLoadingArticle(bool loading);
    void setLoadingComments(bool loading);
    static QVariantMap storyMap(const QVariantMap &json);
    static QString relativeTime(qint64 timestamp);
    static QString hostForUrl(const QString &url);

    QNetworkAccessManager m_network;
    QVariantList m_stories;
    QVector<QVariantMap> m_storySlots;
    QVariantList m_comments;
    QVariantMap m_selectedStory;
    QString m_articleHtml;
    QString m_errorMessage;
    QQueue<CommentRequest> m_commentQueue;
    int m_pendingStories = 0;
    int m_commentCount = 0;
    bool m_loadingStories = false;
    bool m_loadingArticle = false;
    bool m_loadingComments = false;
};
