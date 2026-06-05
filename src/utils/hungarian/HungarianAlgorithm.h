/**
 * @file HungarianAlgorithm.h
 * @brief 匈牙利算法 (Hungarian / Kuhn-Munkres)
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 求解二部图最优分配问题(最小权匹配)。
 * 时间复杂度 O(n^3)，适用于任务调度、目标跟踪等场景。
 */

#ifndef HUNGARIANALGORITHM_H
#define HUNGARIANALGORITHM_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <limits>

/**
 * @class HungarianAlgorithm
 * @brief 匈牙利算法求解器
 *
 * 输入 n x n 代价矩阵，返回最小总代价和分配方案。
 * 矩阵不必是方阵——自动用零填充至方阵。
 */
class HungarianAlgorithm : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalSolves = 0;       ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均单次处理耗时(ms)
    };

    /** @brief 构造匈牙利算法求解器 @param parent 父对象 */
    explicit HungarianAlgorithm(QObject *parent = nullptr);

    /**
     * @brief 求解最优分配
     * @param costMatrix n x m 代价矩阵(行=工人, 列=任务)
     * @return QPair<总代价, 分配数组>
     *         分配数组长度 = 行数, assignments[i] = 工人 i 分配的任务 j(0-indexed)
     *         失败返回 (0.0, {})
     */
    QPair<double, QVector<int>> solve(const QVector<QVector<double>> &costMatrix);

    /** @brief 获取运行时统计 */
    Stats stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param size 矩阵尺寸 @param totalCost 最小总代价 */
    void solveCompleted(int size, double totalCost);

private:
    /**
     * @brief 核心匈牙利算法(方阵版本)
     *
 * 标准增广路径法: 标记行/列，找零元素，更新标号。
     * @param cost n x n 代价矩阵
     * @return 长度为 n 的分配数组
     */
    QVector<int> hungarianCore(QVector<QVector<double>> &cost);

    Stats m_stats;          ///< 统计数据
    QElapsedTimer m_timer;  ///< 计时器
    double m_timeAccum = 0.0; ///< 累计耗时(ms)
};

#endif // HUNGARIANALGORITHM_H
