#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 双连通分量检测器
 *
 * 基于Tarjan算法检测图的双连通分量和割点。
 */
class Biconnected4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalComponentsFound = 0;
        int totalArticulationsFound = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Biconnected4(QObject* parent = nullptr);

    /** @brief 查找所有双连通分量 */
    QVector<QVector<int>> findComponents(const QVector<QVector<int>>& adjacency);

    /** @brief 查找所有割点(关节点) */
    QVector<int> findArticulationPoints(const QVector<QVector<int>>& adjacency) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void componentFound(int componentIndex, int edgeCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
