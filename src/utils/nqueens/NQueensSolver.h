/**
 * @file NQueensSolver.h
 * @brief N皇后问题求解器(N-Queens Solver)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class NQueensSolver
 * @brief N皇后问题求解器 — 回溯+位运算优化
 *
 * 支持求解所有解、统计解个数、验证布局合法性。
 * 使用位运算加速列/对角线冲突检测。
 * 适用于组合优化、约束满足问题、算法教学等场景。
 */
class NQueensSolver : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSolved = 0;       /**< 总求解次数 */
        long long totalSolutions = 0; /**< 总解数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit NQueensSolver(QObject* parent = nullptr);

    /**
     * @brief 求解所有解
     * @param n 棋盘大小(n×n)
     * @param maxSolutions 最大解数(0=全部)
     * @return 所有合法布局, 每个布局为行→列映射
     */
    QVector<QVector<int>> solve(int n, int maxSolutions = 0);

    /**
     * @brief 统计解的个数(不存储解)
     * @param n 棋盘大小
     * @return 解的总数
     */
    long long countSolutions(int n);

    /**
     * @brief 找第一个解
     * @param n 棋盘大小
     * @return 第一个合法布局, 无解返回空
     */
    QVector<int> firstSolution(int n);

    /**
     * @brief 验证布局是否合法
     * @param placement 行→列映射
     * @return 是否合法
     */
    static bool isValid(const QVector<int>& placement);

    /**
     * @brief 转为棋盘字符串表示
     * @param placement 行→列映射
     * @return 棋盘字符串(Q=皇后, .=空)
     */
    static QVector<QString> toBoard(const QVector<int>& placement);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 */
    void solved(int n, long long solutionCount);

private:
    void solveBitmask(int n, int row, int cols, int diag1, int diag2,
                       QVector<QVector<int>>& results, int maxSol);
    void countBitmask(int n, int row, int cols, int diag1, int diag2,
                       long long& count);

    Stats m_stats;
    double m_timeSum;
};
