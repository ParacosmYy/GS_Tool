/**
 * @file BipartiteMatcher.h
 * @brief 二部图匹配引擎 — 简化匈牙利算法求最优匹配
 *
 * 功能: 给定代价矩阵，求解二部图最优匹配（最小权匹配），
 *       返回匹配对及总代价，统计匹配次数与平均耗时。
 *
 * 协作: DataCorrelator(关联匹配) / DataClassifier(分类配对)
 */
#ifndef BIPARTITEMATCHER_H
#define BIPARTITEMATCHER_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二部图匹配器 — 简化匈牙利算法
 */
class BipartiteMatcher : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalMatches = 0;           ///< 累计匹配次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit BipartiteMatcher(QObject* parent = nullptr);

    /**
     * @brief 执行最优匹配
     * @param cost 代价矩阵(cost[行][列])
     * @return 匹配对列表(行索引, 列索引)
     */
    QVector<QPair<int, int>> match(const QVector<QVector<double>>& cost);

    /** @brief 最近一次匹配的总代价 @return 代价 */
    double totalCost() const { return m_totalCost; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 匹配完成信号 @param matchCount 匹配数 @param cost 总代价 */
    void matchingCompleted(int matchCount, double cost);

private:
    /** @brief 尝试为行u分配列 @param u 行索引 @param cost 代价矩阵 */
    bool tryAssign(int u, const QVector<QVector<double>>& cost);

    /**
     * @brief 寻找增广路径
     * @param u 当前行
     * @param cost 代价矩阵
     * @return 是否找到增广路径
     */
    bool findAugmentingPath(int u, const QVector<QVector<double>>& cost);

    double m_totalCost;         ///< 最近一次匹配总代价
    double m_timeSum;           ///< 累计耗时(ms)
    mutable Stats m_stats;      ///< 可变统计

    /* 匹配算法内部状态 */
    QVector<int>    m_rowMatch;     ///< 行匹配到的列(-1未匹配)
    QVector<int>    m_colMatch;     ///< 列匹配到的行(-1未匹配)
    QVector<double> m_rowPot;       ///< 行势值
    QVector<double> m_colPot;       ///< 列势值
    QVector<bool>   m_rowVisited;   ///< 行访问标记
    QVector<bool>   m_colVisited;   ///< 列访问标记
};

#endif // BIPARTITEMATCHER_H
