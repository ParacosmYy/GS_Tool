#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 图同构检测器
 *
 * 基于VF2算法检测两个图是否同构。
 */
class GraphIsomorph4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalComparisons = 0;
        int totalIsomorphicFound = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GraphIsomorph4(QObject* parent = nullptr);

    /** @brief 检测两个图是否同构 */
    bool isIsomorphic(const QVector<QVector<int>>& g1, const QVector<QVector<int>>& g2);

    /** @brief 获取同构映射(顶点对应关系) */
    QVector<int> mapping() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void comparisonCompleted(bool isomorphic, int nodeCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<int> m_mapping;
};
