/**
 * @file MaxBipartiteMatch.h
 * @brief 最大权二部图匹配 — 匈牙利算法实现
 *
 * 功能: 基于匈牙利算法(Kuhn-Munkres)求解最大权二部图完美匹配，
 *       支持任意大小的代价矩阵，使用标号法优化搜索路径。
 *       时间复杂度 O(n^3)，空间复杂度 O(n^2)。
 *
 * 协作: DataDiffWidget(数据关联) / AssignmentOptimizer(任务分配)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
#include <QElapsedTimer>
#include <limits>

/**
 * @brief 匈牙利算法 — 最大/最小权二部图匹配
 */
class MaxBipartiteMatch : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalSolves = 0;           ///< 累计求解次数
        int    totalAugmentingPaths = 0;  ///< 累计增广路径数
        double totalWeight = 0.0;         ///< 累计匹配总权重
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MaxBipartiteMatch(QObject* parent = nullptr);

    /**
     * @brief 求解最大权匹配
     * @param costMatrix n×n代价矩阵 (costMatrix[i][j] = 左顶点i到右顶点j的权重)
     * @return 匹配结果列表 (左索引, 右索引) 及总权重
     */
    QPair<QVector<QPair<int, int>>, double>
    solveMaxWeight(const QVector<QVector<double>>& costMatrix);

    /**
     * @brief 求解最小权匹配
     * @param costMatrix n×n代价矩阵
     * @return 匹配结果列表及总代价
     */
    QPair<QVector<QPair<int, int>>, double>
    solveMinWeight(const QVector<QVector<double>>& costMatrix);

    /**
     * @brief 验证匹配是否为完美匹配
     * @param costMatrix 代价矩阵
     * @param matches 匹配结果
     * @return 是否完美匹配
     */
    bool isPerfectMatch(const QVector<QVector<double>>& costMatrix,
                        const QVector<QPair<int, int>>& matches) const;

    /**
     * @brief 获取对偶变量(标号)用于调试
     * @return (u标号数组, v标号数组)
     */
    QPair<QVector<double>, QVector<double>> labels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param matchCount 匹配数 @param totalWeight 总权重 */
    void solveCompleted(int matchCount, double totalWeight);

private:
    /**
     * @brief 匈牙利算法核心(最小化版本)
     * @param mat 代价矩阵(将被修改)
     * @return 匹配数组 matchR[j] = i 表示右j匹配左i
     */
    QVector<int> hungarianCore(QVector<QVector<double>>& mat);

    /** @brief 寻找增广路径(DFS) */
    bool findAugmentingPath(int u, const QVector<QVector<double>>& mat,
                            QVector<bool>& visitedL, QVector<bool>& visitedR,
                            QVector<double>& uLabel, QVector<double>& vLabel,
                            QVector<int>& matchL, QVector<int>& matchR,
                            QVector<double>& slack, QVector<int>& slackFrom);

    /** @brief 更新标号 */
    void updateLabels(int n, const QVector<bool>& visitedL,
                      const QVector<bool>& visitedR,
                      QVector<double>& uLabel, QVector<double>& vLabel,
                      const QVector<double>& slack);

    Stats              m_stats;
    double             m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;

    /* 保留最近一次求解的标号 */
    QVector<double>    m_uLabels;
    QVector<double>    m_vLabels;
};
