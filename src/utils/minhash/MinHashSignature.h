/**
 * @file MinHashSignature.h
 * @brief MinHash签名 — 集合相似度快速估计
 *
 * 功能: 使用MinHash算法估计Jaccard相似度，支持批量哈希、
 *       集合比较、Top-K相似查询，统计哈希次数/比较次数/耗时。
 */
#ifndef MINHASHSIGNATURE_H
#define MINHASHSIGNATURE_H

#include <QObject>
#include <QVector>
#include <QSet>
#include <QByteArray>

class MinHashSignature : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalHashes = 0;
        quint64 totalComparisons = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MinHashSignature(int numHashes = 128,
                               QObject* parent = nullptr);

    /** @brief 计算集合的MinHash签名 @param items 集合元素 @return 签名向量 */
    QVector<quint32> computeSignature(const QSet<QByteArray>& items);

    /** @brief 估计Jaccard相似度 @param sig1 签名1 @param sig2 签名2 @return 相似度[0,1] */
    double jaccardSimilarity(const QVector<quint32>& sig1,
                             const QVector<quint32>& sig2) const;

    /** @brief 批量计算签名 @param itemSets 集合列表 @return 签名列表 */
    QVector<QVector<quint32>> batchSignatures(
        const QVector<QSet<QByteArray>>& itemSets);

    /** @brief Top-K最相似 @param query 查询签名 @param candidates 候选签名 @param k Top-K @return 索引+相似度列表 */
    QVector<QPair<int, double>> topKSimilar(
        const QVector<quint32>& query,
        const QVector<QVector<quint32>>& candidates, int k) const;

    int numHashes() const { return m_numHashes; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void signatureComputed(int setSize);
    void comparisonCompleted(double similarity);

private:
    quint32 hashItem(const QByteArray& item, int hashIdx) const;
    quint32 murmurHash(const QByteArray& data, quint32 seed) const;

    int m_numHashes;
    Stats m_stats;
    double m_timeSum;
};

#endif // MINHASHSIGNATURE_H
