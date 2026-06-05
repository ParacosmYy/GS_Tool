#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 拓扑排序算法
 *
 * 有向无环图(DAG)的拓扑排序，支持环检测。
 */
class TopologicalSort4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSorts = 0;
        int totalCyclesDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TopologicalSort4(QObject* parent = nullptr);

    /** @brief Kahn算法拓扑排序 */
    QVector<int> sort(const QVector<QVector<int>>& adjacency);

    /** @brief 检测图中是否存在环 */
    bool hasCycle(const QVector<QVector<int>>& adjacency) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sortCompleted(int nodeCount, bool hasCycle);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
