/**
 * @file LeastSquaresSolver.h
 * @brief 加权最小二乘求解器 — Weighted Least Squares via QR Decomposition
 *
 * 功能: 基于QR分解的加权最小二乘法求解器。支持列满秩和亏秩情况，
 *       提供残差分析、条件数计算和置信区间估计。
 *       适用于线性回归、曲线拟合和参数估计。
 *
 * 协作: SvdDecomposer(SVD分解) / EigenSolver(特征值) / CholeskyDecomposer(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 加权最小二乘求解器
 */
class LeastSquaresSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 求解结果 */
    struct SolveResult {
        QVector<double> coefficients;   ///< 回归系数
        double residualSumSquares = 0.0; ///< 残差平方和
        double rSquared = 0.0;          ///< R²决定系数
        double adjustedRSquared = 0.0;  ///< 调整R²
        double conditionNumber = 0.0;   ///< 条件数
        double standardError = 0.0;     ///< 标准误差
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double lastRSS = 0.0;               ///< 最近残差平方和
    };

    explicit LeastSquaresSolver(QObject* parent = nullptr);

    /**
     * @brief 求解普通最小二乘(Ax = b)
     * @param A 系数矩阵(m x n)
     * @param b 观测向量(m)
     * @return 求解结果
     */
    SolveResult solve(const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /**
     * @brief 求解加权最小二乘
     * @param A 系数矩阵
     * @param b 观测向量
     * @param weights 权重向量(对角加权矩阵)
     * @return 求解结果
     */
    SolveResult solveWeighted(const QVector<QVector<double>>& A,
                               const QVector<double>& b,
                               const QVector<double>& weights);

    /**
     * @brief 多项式拟合
     * @param x 自变量
     * @param y 因变量
     * @param degree 多项式阶数
     * @return 求解结果(系数从低次到高次)
     */
    SolveResult polyFit(const QVector<double>& x,
                         const QVector<double>& y, int degree);

    /**
     * @brief 用拟合结果预测
     * @param coeffs 回归系数
     * @param x 输入值(单变量多项式)或特征向量
     * @return 预测值
     */
    static double predict(const QVector<double>& coeffs, double x);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param n 变量数 @param rSquared R²值 */
    void solveCompleted(int n, double rSquared);

private:
    /** @brief Modified Gram-Schmidt QR分解 */
    bool qrDecompose(QVector<QVector<double>>& A,
                      QVector<QVector<double>>& Q,
                      QVector<QVector<double>>& R) const;

    /** @brief 计算残差统计 */
    SolveResult computeResult(const QVector<QVector<double>>& Q,
                               const QVector<QVector<double>>& R,
                               const QVector<double>& b,
                               int n) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
