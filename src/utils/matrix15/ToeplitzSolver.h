/**
 * @file ToeplitzSolver.h
 * @brief Toeplitz 矩阵求解器 — Levinson-Durbin 递归算法
 *
 * 功能: 高效求解 Toeplitz 线性方程组 T·x = b，时间复杂度 O(n^2)，
 *       支持 Yule-Walker 方程求解、线性预测编码 (LPC) 系数计算、
 *       自回归 (AR) 模型参数估计等场景。
 *
 * 协作: SpectrumAnalyzer(谱分析) / AutoCorrelator(自相关)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Toeplitz 矩阵求解器
 *
 * Toeplitz 矩阵是对角线元素相同的方阵，仅由第一行和第一列定义。
 * Levinson-Durbin 递归利用其结构特性将 O(n^3) 的高斯消元
 * 降至 O(n^2)，同时保证数值稳定性。
 */
class ToeplitzSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 求解结果 */
    struct SolveResult {
        QVector<double> solution;     ///< 解向量 x
        double reflectionCoeff = 0.0; ///< 最终反射系数
        double predictionError = 0.0; ///< 预测误差功率
        bool success = false;         ///< 是否求解成功
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSolves = 0;              ///< 累计求解次数
        int totalDimensions = 0;          ///< 累计矩阵维度
        int totalFailures = 0;            ///< 累计失败次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit ToeplitzSolver(QObject* parent = nullptr);

    /**
     * @brief 求解 Toeplitz 方程组 T·x = b
     * @param firstRow Toeplitz 矩阵第一行 [t0, t1, ..., tn-1]
     * @param rhs 右端向量 b
     * @return 求解结果 (解向量 + 反射系数 + 预测误差)
     */
    SolveResult solve(const QVector<double>& firstRow,
                      const QVector<double>& rhs);

    /**
     * @brief 求解 Yule-Walker 方程 (自相关 -> AR 系数)
     * @param autocorrelation 自相关序列 [r0, r1, ..., rp]
     * @return AR 模型系数 [a1, a2, ..., ap] 和预测误差功率
     */
    SolveResult solveYuleWalker(const QVector<double>& autocorrelation);

    /**
     * @brief 计算反射系数 (PARCOR 系数)
     * @param autocorrelation 自相关序列
     * @return 各阶反射系数 [k1, k2, ..., kp]
     */
    QVector<double> reflectionCoefficients(const QVector<double>& autocorrelation);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param dimension 矩阵维度 @param error 预测误差 */
    void solveCompleted(int dimension, double error);

private:
    Stats m_stats;                ///< 统计信息
    double m_timeSum = 0.0;       ///< 处理时间累加器
};
