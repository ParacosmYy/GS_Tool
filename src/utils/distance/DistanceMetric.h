/**
 * @file DistanceMetric.h
 * @brief 距离度量库 — 多种距离/相似度计算
 *
 * 功能: 提供欧氏/曼哈顿/切比雪夫/闵可夫斯基/余弦/马氏/汉明/
 *       Jaccard/Edit距离等多种度量。用于聚类、分类和模式匹配。
 *
 * 协作: CosineDist(余弦) / KMeansClusterer(聚类)
 */
#ifndef DISTANCEMETRIC_H
#define DISTANCEMETRIC_H

#include <QObject>
#include <QVector>

/**
 * @brief 距离度量库
 */
class DistanceMetric : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;   ///< 累计计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit DistanceMetric(QObject* parent = nullptr);

    /** @brief 欧氏距离 */
    double euclidean(const QVector<double>& a,
                     const QVector<double>& b) const;

    /** @brief 曼哈顿距离 */
    double manhattan(const QVector<double>& a,
                     const QVector<double>& b) const;

    /** @brief 切比雪夫距离 */
    double chebyshev(const QVector<double>& a,
                     const QVector<double>& b) const;

    /** @brief 闵可夫斯基距离 @param p 参数(p=2欧氏, p=1曼哈顿) */
    double minkowski(const QVector<double>& a,
                     const QVector<double>& b, double p) const;

    /** @brief 余弦相似度 */
    double cosineSimilarity(const QVector<double>& a,
                            const QVector<double>& b) const;

    /** @brief 余弦距离(1-相似度) */
    double cosineDistance(const QVector<double>& a,
                         const QVector<double>& b) const;

    /** @brief 汉明距离(等长序列不同位置数) */
    int hamming(const QVector<int>& a, const QVector<int>& b) const;

    /** @brief Jaccard相似系数 */
    double jaccard(const QVector<int>& setA, const QVector<int>& setB) const;

    /** @brief 编辑距离(Levenshtein) */
    int editDistance(const QByteArray& s1, const QByteArray& s2) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param metric 度量名 */
    void computationCompleted(const QString& metric);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // DISTANCEMETRIC_H
