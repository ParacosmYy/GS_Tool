/**
 * @file IterativeRefinement.h
 * @brief 迭代精化求解(混合精度残差+精度提升) — Iterative Refinement for Improving Solution Accuracy with Mixed-Precision Residual Computation
 *
 * 功能: 实现迭代精化方法，支持低精度求解+高精度残差修正、
 *       收敛监控、条件数估计和解误差追踪。
 *
 * 协作: GaussianElimination5(高斯消元) / LUDecomposition4(LU分解) / Cholesky4(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 迭代精化求解器(混合精度残差修正)
 */
class IterativeRefinement : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IterativeRefinement(QObject *parent = nullptr);
    ~IterativeRefinement() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief 求解Ax=b，返回x */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief 获取残差历史 */
    QVector<double> residualHistory() const { return m_residualHistory; }

    /** @brief 估计矩阵条件数(1-范数) */
    double estimateConditionNumber(const QVector<QVector<double>>& A) const;

    /** @brief 计算残差 r = b - Ax */
    QVector<double> computeResidual(const QVector<QVector<double>>& A,
                                    const QVector<double>& x,
                                    const QVector<double>& b) const;

    /** @brief 向量范数(2-范数) */
    double vectorNorm(const QVector<double>& v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double finalResidual, double timeMs);

private:
    int m_maxIterations = 50;
    double m_tolerance = 1e-12;

    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_residualHistory;

    /** @brief LU decomposition (Doolittle) */
    bool luDecompose(QVector<QVector<double>>& A,
                     QVector<int>& perm) const;

    /** @brief LU forward/back substitution */
    QVector<double> luSolve(const QVector<QVector<double>>& LU,
                            const QVector<int>& perm,
                            const QVector<double>& b) const;

    /** @brief Matrix-vector multiply */
    QVector<double> matVecMultiply(const QVector<QVector<double>>& A,
                                   const QVector<double>& x) const;

    /** @brief Matrix 1-norm */
    double matrixNorm1(const QVector<QVector<double>>& A) const;

    /** @brief Matrix inverse 1-norm estimate (Hager's method) */
    double estimateInverseNorm1(const QVector<QVector<double>>& A) const;
};
