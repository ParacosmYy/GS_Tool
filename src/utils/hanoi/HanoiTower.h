/**
 * @file HanoiTower.h
 * @brief 汉诺塔求解器(Hanoi Tower Solver)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class HanoiTower
 * @brief 汉诺塔求解器 — 经典递归/迭代/Frame-Stewart算法
 *
 * 支持3柱/4柱汉诺塔求解、最优步数计算、步骤可视化。
 * 适用于算法教学、递归分析、状态空间搜索等场景。
 */
class HanoiTower : public QObject
{
    Q_OBJECT

public:
    /** @brief 一步移动 */
    struct Move {
        int disk;        /**< 盘子编号(1=最小) */
        int from;        /**< 源柱 */
        int to;          /**< 目标柱 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSolved = 0;       /**< 总求解次数 */
        long long totalMoves = 0;  /**< 总移动步数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit HanoiTower(QObject* parent = nullptr);

    /**
     * @brief 经典3柱汉诺塔求解
     * @param n 盘子数
     * @return 移动步骤列表
     */
    QVector<Move> solve(int n);

    /**
     * @brief 4柱Frame-Stewart算法
     * @param n 盘子数
     * @return 移动步骤列表
     */
    QVector<Move> solveFourPegs(int n);

    /**
     * @brief 计算最优移动步数(不实际求解)
     * @param n 盘子数
     * @param pegs 柱子数(3或4)
     * @return 最优步数
     */
    static long long optimalMoves(int n, int pegs = 3);

    /**
     * @brief 验证移动序列是否合法
     * @param moves 移动序列
     * @param n 盘子数
     * @param pegs 柱子数
     * @return 是否合法
     */
    static bool validateMoves(const QVector<Move>& moves, int n, int pegs = 3);

    /**
     * @brief 获取当前状态表示
     * @param n 盘子数
     * @param pegs 柱子数
     * @param moves 已执行的移动
     * @return 每根柱子上的盘子列表(从底到顶)
     */
    static QVector<QVector<int>> stateAfter(int n, int pegs,
                                              const QVector<Move>& moves);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 */
    void solved(int disks, long long moves);

private:
    void solveRecursive(int n, int from, int to, int aux,
                         QVector<Move>& moves);
    void solveFrameStewart(int n, int from, int to, int aux1, int aux2,
                            QVector<Move>& moves);

    Stats m_stats;
    double m_timeSum;
};
