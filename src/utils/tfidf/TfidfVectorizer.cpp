/**
 * @file TfidfVectorizer.cpp
 * @brief TF-IDF向量化器 — 文本特征提取与相似度检索
 */

#include "TfidfVectorizer.h"
#include <QElapsedTimer>
#include <QRegularExpression>
#include <cmath>

TfidfVectorizer::TfidfVectorizer(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void TfidfVectorizer::addDocument(const QString& docId, const QStringList& terms)
{
    QElapsedTimer timer;
    timer.start();

    /* 词频统计 */
    QMap<QString, int>& tf = m_termFreq[docId];
    for (const QString& term : terms) {
        tf[term]++;
    }

    /* 更新文档频率 */
    QStringList uniqueTerms;
    for (const QString& term : terms) {
        if (!uniqueTerms.contains(term)) uniqueTerms.append(term);
    }
    for (const QString& term : uniqueTerms) {
        m_docFreq[term]++;
    }

    m_stats.totalDocumentsIndexed++;
    m_stats.vocabularySize = static_cast<quint64>(m_docFreq.size());
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDocumentsIndexed + m_stats.totalQueries);

    emit documentIndexed(docId, terms.size());
}

void TfidfVectorizer::addDocument(const QString& docId, const QString& text)
{
    QStringList terms = text.split(
        QRegularExpression(QStringLiteral(R"([\s,.\-;:!?/\\()\[\]{}'"<>]+)")),
        Qt::SkipEmptyParts);
    addDocument(docId, terms);
}

QMap<QString, double> TfidfVectorizer::tfidfVector(const QString& docId) const
{
    QMap<QString, double> result;
    if (!m_termFreq.contains(docId)) return result;

    const QMap<QString, int>& tf = m_termFreq[docId];
    int totalDocs = documentCount();

    /* 计算文档总词数 */
    int totalTerms = 0;
    for (auto it = tf.constBegin(); it != tf.constEnd(); ++it) {
        totalTerms += it.value();
    }

    for (auto it = tf.constBegin(); it != tf.constEnd(); ++it) {
        const QString& term = it.key();
        double tfVal = static_cast<double>(it.value()) / totalTerms;
        double idfVal = std::log(static_cast<double>(totalDocs) /
                                  (1 + m_docFreq.value(term, 0)));
        result[term] = tfVal * idfVal;
    }

    return result;
}

QVector<QPair<QString, double>> TfidfVectorizer::query(
    const QStringList& queryTerms, int topK) const
{
    QElapsedTimer timer;
    timer.start();

    /* 构造查询的TF-IDF向量 */
    QMap<QString, int> queryTf;
    for (const QString& term : queryTerms) {
        queryTf[term]++;
    }

    int totalDocs = documentCount();
    QMap<QString, double> queryVec;
    int queryTotal = queryTerms.size();
    for (auto it = queryTf.constBegin(); it != queryTf.constEnd(); ++it) {
        double tfVal = static_cast<double>(it.value()) / queryTotal;
        double idfVal = std::log(static_cast<double>(totalDocs) /
                                  (1 + m_docFreq.value(it.key(), 0)));
        queryVec[it.key()] = tfVal * idfVal;
    }

    /* 与所有文档计算余弦相似度 */
    QVector<QPair<QString, double>> scores;
    for (auto it = m_termFreq.constBegin(); it != m_termFreq.constEnd(); ++it) {
        QMap<QString, double> docVec = tfidfVector(it.key());
        double sim = cosineSimilarity(queryVec, docVec);
        scores.append({it.key(), sim});
    }

    /* 按相似度降序排序 */
    std::sort(scores.begin(), scores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (scores.size() > topK) scores.resize(topK);

    const_cast<TfidfVectorizer*>(this)->m_stats.totalQueries++;
    const_cast<TfidfVectorizer*>(this)->m_timeSum += timer.elapsed();
    const_cast<TfidfVectorizer*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalDocumentsIndexed + m_stats.totalQueries);

    const_cast<TfidfVectorizer*>(this)->queryCompleted(scores.size());
    return scores;
}

double TfidfVectorizer::similarity(const QString& doc1, const QString& doc2) const
{
    QMap<QString, double> v1 = tfidfVector(doc1);
    QMap<QString, double> v2 = tfidfVector(doc2);
    return cosineSimilarity(v1, v2);
}

QStringList TfidfVectorizer::vocabulary() const
{
    return m_docFreq.keys();
}

double TfidfVectorizer::cosineSimilarity(const QMap<QString, double>& v1,
                                           const QMap<QString, double>& v2) const
{
    double dotProduct = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;

    for (auto it = v1.constBegin(); it != v1.constEnd(); ++it) {
        norm1 += it.value() * it.value();
        if (v2.contains(it.key())) {
            dotProduct += it.value() * v2[it.key()];
        }
    }

    for (auto it = v2.constBegin(); it != v2.constEnd(); ++it) {
        norm2 += it.value() * it.value();
    }

    double denom = std::sqrt(norm1) * std::sqrt(norm2);
    return (denom > 1e-15) ? dotProduct / denom : 0.0;
}

void TfidfVectorizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
