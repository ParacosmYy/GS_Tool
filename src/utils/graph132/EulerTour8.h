#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief EulerTour8 - 欧拉回路/路径第8代实现
 *
 * 提供欧拉回路和欧拉路径的判定与构造算法，
 * 支持Fleury算法和Hierholzer算法。
 */
class EulerTour8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit EulerTour8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Hierholzer算法构造欧拉回路
     * @param adjacencyList 图的邻接表
     * @return 欧拉回路的顶点序列，不存在则返回空
     */
    QVector<int> hierholzerCircuit(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief 构造欧拉路径
     * @param adjacencyList 图的邻接表
     * @return 欧拉路径的顶点序列，不存在则返回空
     */
    QVector<int> eulerPath(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief 检查图是否存在欧拉回路
     * @param adjacencyList 图的邻接表
     * @param isDirected 是否为有向图
     * @return 是否存在欧拉回路
     */
    bool hasEulerCircuit(const QVector<QVector<int>>& adjacencyList,
                         bool isDirected = false) const;

    /**
     * @brief 计算各顶点的度数
     * @param adjacencyList 图的邻接表
     * @param isDirected 是否为有向图
     * @return 各顶点的度数（有向图返回出度-入度差）
     */
    QVector<int> computeDegrees(const QVector<QVector<int>>& adjacencyList,
                                bool isDirected = false) const;

signals:
    void solveCompleted(int pathLength);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
