/**
 * @file MaximumMatching.h
 * @brief 一般图最大匹配 — Edmonds花朵+加权匹配
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 一般图最大/最大加权匹配
 * Blossom算法(无权) + Hungarian风格(加权)
 */
class MaximumMatching : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalMatches = 0;
        int totalAugmentations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MaximumMatching(QObject* parent = nullptr);

    /** @brief 设置无权图 @param edges 边列表(u,v) @param n 节点数 */
    void setUnweighted(const QVector<QPair<int,int>>& edges, int n);

    /** @brief 设置加权图 @param edges 带权边(u,v,w) @param n 节点数 */
    void setWeighted(const QVector<QPair<QPair<int,int>,double>>& edges, int n);

    /** @brief 求最大匹配(无权) @return 匹配边列表 */
    QVector<QPair<int,int>> maximumMatching();

    /** @brief 求最大加权匹配 @return 匹配边列表 */
    QVector<QPair<int,int>> maximumWeightedMatching();

    /** @brief 获取匹配大小 */
    int matchingSize() const { return m_matching.size(); }

    /** @brief 获取匹配总权重(加权匹配) */
    double matchingWeight() const { return m_totalWeight; }

    /** @brief 判断完美匹配 */
    bool isPerfect() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void matchingFound(int size, double weight);

private:
    int m_n = 0;
    QVector<QPair<int,int>> m_matching;
    double m_totalWeight = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
