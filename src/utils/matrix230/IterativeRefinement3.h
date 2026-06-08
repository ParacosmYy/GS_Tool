/**
 * @file IterativeRefinement3.h
 * @brief 迭代精化(混合精度缺陷校正+超精度残差累加) — Iterative Refinement with Defect Correction in Mixed Precision and Extra-Precise Residual Accumulation
 *
 * 功能: 实现混合精度迭代精化算法，低精度求解+高精度残差计算，
 *       使用超精度(double)累加残差以提升数值解精度。
 *
 * 协作: LUDecomposition6(LU分解) / QRDecomposition5(QR分解) / Cholesky3(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 迭代精化(混合精度缺陷校正)
 */
class IterativeRefinement3 : public QObject {
    Q_OBJECT

public:
    /** @brief Refinement result */
    struct RefinementResult {
        QVector<double> solution;
        double initialResidual = 0.0;
        double finalResidual = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int maxIterations = 20;
        double tolerance = 1e-12;
        int totalRefinements = 0;
        int totalConverged = 0;
        double avgIterations = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IterativeRefinement3(QObject *parent = nullptr);
    ~IterativeRefinement3() override;

    /** @brief Set refinement parameters */
    void setParameters(double tolerance = 1e-12, int maxIterations = 20);

    /** @brief Solve Ax = b with iterative refinement */
    RefinementResult solve(const QVector<QVector<double>>& A,
                           const QVector<double>& b);

    /** @brief Compute residual r = b - Ax in extra precision */
    QVector<double> computeResidual(
        const QVector<QVector<double>>& A,
        const QVector<double>& x,
        const QVector<double>& b) const;

    /** @brief Compute infinity-norm of vector */
    double normInf(const QVector<double>& v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void refinementCompleted(int iterations, double finalResidual, double timeMs);

private:
    double m_tolerance = 1e-12;
    int m_maxIterations = 20;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief LU decomposition with partial pivoting (working precision) */
    bool luDecompose(QVector<QVector<double>>& A,
                     QVector<int>& pivot) const;

    /** @brief LU forward/backward solve (working precision) */
    QVector<double> luSolve(const QVector<QVector<double>>& LU,
                            const QVector<int>& pivot,
                            const QVector<double>& rhs) const;

    /** @brief Matrix-vector multiply y = A*x (extra precision) */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                           const QVector<double>& x) const;

    /** @brief Kahan compensated summation for dot product */
    double compensatedDot(const QVector<double>& row,
                          const QVector<double>& x) const;
};
