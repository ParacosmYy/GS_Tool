/**
 * @file EdmondsMatching.h
 * @brief Edmonds 花算法 (Blossom Algorithm) 最大匹配
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现简化版 Edmonds 花算法，求解一般图的最大基数匹配。
 * 使用广度优先搜索交替路径，遇到花(blossom)时收缩。
 * 时间复杂度 O(n^2 m)，适用于中小规模图。
 */

#ifndef EDMONDSMATCHING_H
#define EDMONDSMATCHING_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <QQueue>
#include <vector>

/**
 * @class EdmondsMatching
 * @brief Edmonds 花算法求解器
 *
 * 输入边列表和顶点数，返回最大匹配的边集合。
 */
class EdmondsMatching : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalSolves = 0;       ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均单次处理耗时(ms)
    };

    /** @brief 构造 Edmonds 匹配求解器 @param parent 父对象 */
    explicit EdmondsMatching(QObject *parent = nullptr);

    /**
     * @brief 求解最大匹配
     * @param edges 边列表(每条边为顶点对, 0-indexed)
     * @param n 顶点总数
     * @return 最大匹配的边列表
     *
     * 若 n <= 0 或无有效边返回空列表。
     */
    QVector<QPair<int, int>> maxMatching(const QVector<QPair<int, int>> &edges, int n);

    /** @brief 获取运行时统计 */
    Stats stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param matchingSize 匹配边数 */
    void solveCompleted(int matchingSize);

private:
    /**
     * @brief 查找顶点 u 在并查集中的根(带路径压缩)
     * @param parent 并查集父数组
     * @param u 顶点
     * @return 根顶点
     */
    static int findRoot(QVector<int> &parent, int u);

    /**
     * @brief 从花中提取实际路径
     * @param u 起始顶点
     * @param v 终止顶点
     * @param blossomMark 花标记数组
     * @param base 基顶点
     * @param match 匹配数组
     * @param path 路径输出
     */
    static void extractPath(int u, int v, const QVector<int> &blossomMark,
                            int base, const QVector<int> &match,
                            QVector<int> &path);

    Stats m_stats;          ///< 统计数据
    QElapsedTimer m_timer;  ///< 计时器
    double m_timeAccum = 0.0; ///< 累计耗时(ms)
};

#endif // EDMONDSMATCHING_H
