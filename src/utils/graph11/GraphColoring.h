/**
 * @file GraphColoring.h
 * @brief 图着色(Graph Coloring)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class GraphColoring
 * @brief 图着色 — 贪心/Dsatur图着色算法
 *
 * 支持贪心/DSATUR/回溯着色、色数下界估计。
 * 适用于资源分配、寄存器分配、调度问题等场景。
 */
class GraphColoring : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalColored = 0;       /**< 总着色次数 */
        int totalColors = 0;        /**< 总使用颜色数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit GraphColoring(QObject* parent = nullptr);

    /**
     * @brief 贪心着色(顺序)
     * @param adjacency 邻接表
     * @return 每个顶点的颜色(0-based)
     */
    QVector<int> greedy(const QVector<QVector<int>>& adjacency);

    /**
     * @brief DSATUR着色(度饱和)
     * @param adjacency 邻接表
     * @return 每个顶点的颜色
     */
    QVector<int> dsatur(const QVector<QVector<int>>& adjacency);

    /**
     * @brief Welsh-Powell着色(按度排序)
     * @param adjacency 邻接表
     * @return 每个顶点的颜色
     */
    QVector<int> welshPowell(const QVector<QVector<int>>& adjacency);

    /**
     * @brief 检查着色是否合法
     * @param adjacency 邻接表
     * @param colors 颜色分配
     * @return 是否合法
     */
    static bool isValid(const QVector<QVector<int>>& adjacency,
                          const QVector<int>& colors);

    /**
     * @brief 计算使用的颜色数
     * @param colors 颜色分配
     * @return 颜色数
     */
    static int colorCount(const QVector<int>& colors);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 着色完成信号 */
    void coloringCompleted(int vertices, int colors);

private:
    Stats m_stats;
    double m_timeSum;
};
