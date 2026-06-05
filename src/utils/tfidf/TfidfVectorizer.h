/**
 * @file TfidfVectorizer.h
 * @brief TF-IDF向量化器 — 文本特征提取
 *
 * 功能: 计算TF-IDF(Term Frequency-Inverse Document Frequency)，
 *       支持文档集统计、查询向量化、余弦相似度检索，
 *       统计文档数/词汇数/耗时。
 */
#ifndef TFIDFVECTORIZER_H
#define TFIDFVECTORIZER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QStringList>

class TfidfVectorizer : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalDocumentsIndexed = 0;
        quint64 totalQueries = 0;
        quint64 vocabularySize = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit TfidfVectorizer(QObject* parent = nullptr);

    /** @brief 添加文档 @param docId 文档ID @param terms 词语列表 */
    void addDocument(const QString& docId, const QStringList& terms);

    /** @brief 添加文档(自动分词) @param docId 文档ID @param text 文本 */
    void addDocument(const QString& docId, const QString& text);

    /** @brief 计算TF-IDF向量 @param docId 文档ID @return (词→TF-IDF)映射 */
    QMap<QString, double> tfidfVector(const QString& docId) const;

    /** @brief 查询最相似文档 @param queryTerms 查询词 @param topK 返回数 @return (docId, 相似度)列表 */
    QVector<QPair<QString, double>> query(const QStringList& queryTerms,
                                           int topK = 10) const;

    /** @brief 两个文档的余弦相似度 @param doc1 文档1 @param doc2 文档2 @return 相似度[0,1] */
    double similarity(const QString& doc1, const QString& doc2) const;

    int documentCount() const { return m_docFreq.size(); }
    QStringList vocabulary() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void documentIndexed(const QString& docId, int termCount);
    void queryCompleted(int resultCount);

private:
    double cosineSimilarity(const QMap<QString, double>& v1,
                            const QMap<QString, double>& v2) const;

    QMap<QString, int> m_docFreq;       ///< 词→包含该词的文档数
    QMap<QString, QMap<QString, int>> m_termFreq; ///< docId→(term→count)
    Stats m_stats;
    double m_timeSum;
};

#endif // TFIDFVECTORIZER_H
