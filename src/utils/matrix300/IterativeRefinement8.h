/**
 * @file IterativeRefinement8.h
 * @brief 迭代精化(混合精度残差计算与条件数估计实现高精度线性求解) — Iterative Refinement with Mixed-precision Residual Computation and Condition Number Estimation for High-accuracy Linear Solve
 *
 * 功能: 实现迭代精化(iterative refinement)，采用混合精度残差计算(mixed-precision residual computation)
 *       与条件数估计(condition number estimation)实现高精度线性求解(high-accuracy linear solve)。
 *
 * 协作: LUDecomposition10(LU分解) / QRDecomposition11(QR分解) / GaussSeidel8(高斯-赛德尔迭代)
 */
#pragma once

#include <QObject>
#include <QVector>

class IterativeRefinement8 : public QObject {
    Q_OBJECT

public:
    /** @brief Solve result */
    struct SolveResult {
        QVector<double> solution;
        double residualNorm = 0.0;
        double conditionNumber = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        double avgIterations = 0.0;
        double avgConditionNumber = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IterativeRefinement8(QObject *parent = nullptr);
    ~IterativeRefinement8() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief Solve Ax=b with iterative refinement using LU factorization */
    SolveResult solve(const QVector<QVector<double>>& A,
                       const QVector<double>& b);

    /** @brief Compute condition number estimate (1-norm) */
    double estimateConditionNumber(const QVector<QVector<double>>& A) const;

    /** @brief Compute residual r = b - Ax */
    QVector<double> computeResidual(const QVector<QVector<double>>& A,
                                     const QVector<double>& x,
                                     const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, int iters, double cond, double timeMs);

private:
    int m_maxIter = 20;
    double m_tolerance = 1e-12;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_iterSum = 0.0;
    double m_condSum = 0.0;

    /** @brief LU factorization with partial pivoting */
    struct LUResult {
        QVector<QVector<double>> LU;
        QVector<int> perm;
    };

    LUResult luDecompose(const QVector<QVector<double>>& A) const;

    /** @brief Solve LUx = b via forward/back substitution */
    QVector<double> luSolve(const LUResult& lu,
                             const QVector<double>& b) const;

    /** @brief Compute 1-norm of matrix */
    double matrixNorm1(const QVector<QVector<double>>& A) const;

    /** @brief Compute 2-norm of vector */
    double vectorNorm2(const QVector<double>& v) const;
};
