#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DominatingSet4 - 最小支配集求解器
 *
 * 寻找图的最小支配集(每个顶点要么在集合中要么
 * 与集合中顶点相邻)，使用贪心近似和精确算法。
 */
class DominatingSet4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSetsComputed = 0;
        int totalVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DominatingSet4(QObject* parent = nullptr);

    /** @brief 使用贪心近似求解最小支配集 */
    QVector<int> greedySolve(const QVector<QVector<int>>& adjacency);

    /** @brief 验证给定集合是否为支配集 */
    bool isDominating(const QVector<int>& set,
                      const QVector<QVector<int>>& adjacency) const;

    /** @brief 获取支配集大小 */
    int setSize() const;

    /** @brief 获取未被支配的顶点(验证用) */
    QVector<int> undominated(const QVector<int>& set,
                             const QVector<QVector<int>>& adjacency) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void setComputed(int setSize, int vertexCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_setSize = 0;
    QVector<int> m_solution;
};
