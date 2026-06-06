/**
 * @file Hungarian.h
 * @brief 匈牙利算法(最小权完美二分匹配) — Hungarian Algorithm for Minimum-Weight Perfect Bipartite Matching
 *
 * 功能: 实现匈牙利算法(Kuhn-Munkres)，求解二分图最小权完美匹配。
 *       支持方阵代价矩阵，O(n^3)复杂度，基于标号法(labelling)。
 *
 * 协作: BipartiteMatcher(二分图匹配) / FloydWarshall(最短路) / DenseGraph(图结构)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 匈牙利算法求解器
 */
class Hungarian : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;           ///< 累计求解次数
        int lastMatrixSize = 0;            ///< 最近矩阵维度
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        double lastTotalCost = 0.0;        ///< 最近总代价
    };

    explicit Hungarian(QObject* parent = nullptr);
    ~Hungarian() override;

    /**
     * @brief 求解最小权完美匹配
     * @param costMatrix n×n代价矩阵
     * @return 每行对应的列匹配(col[j]=i 表示列j匹配行i)，-1表示未匹配
     */
    QVector<int> solve(const QVector<QVector<double>>& costMatrix);

    /**
     * @brief 计算匹配总代价
     * @param costMatrix 代价矩阵
     * @param assignment 匹配结果
     * @return 总代价
     */
    double totalCost(const QVector<QVector<double>>& costMatrix,
                     const QVector<int>& assignment) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param n 矩阵维度 @param cost 总代价 */
    void solveCompleted(int n, double cost);

private:
    /** @brief 寻找增广路径 */
    bool findAugmentingPath(int row, const QVector<QVector<double>>& cost,
                            QVector<double>& rowLabel, QVector<double>& colLabel,
                            QVector<int>& rowMatch, QVector<int>& colMatch,
                            QVector<bool>& rowVisited, QVector<bool>& colVisited,
                            QVector<double>& slack, QVector<int>& slackRow) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
