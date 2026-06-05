#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 图支配集求解器
 *
 * 寻找图的最小支配集，支持贪心近似和精确求解。
 */
class DominatingSet5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSetsComputed = 0;
        int totalGraphsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet5(QObject* parent = nullptr);

    /** @brief 贪心近似求解最小支配集 */
    QVector<int> greedyApprox(const QVector<QVector<int>>& adjacency);

    /** @brief 验证集合是否为有效支配集 */
    bool validate(const QVector<QVector<int>>& adjacency, const QVector<int>& set) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void setComputed(int setSize, bool isOptimal);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
