/**
 * @file ConjugateGradient5.h
 * @brief 共轭梯度法(IC(0)预处理+CGS平方变体求解非对称系统) — Conjugate Gradient with IC(0) Preconditioner and CGS Squared Variant for Non-Symmetric Systems
 *
 * 功能: 实现共轭梯度求解器，包含IC(0)不完全Cholesky预处理，
 *       CGS平方变体支持非对称矩阵系统的高效求解。
 *
 * 协作: SparseMatrix5(稀疏矩阵) / LUDecomposition4(LU分解) / IterativeSolver3(迭代求解器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度法(IC(0)预处理+CGS)
 */
class ConjugateGradient5 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        int iterations = 0;
        double residual = 0.0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int maxIterations = 0;
        double tolerance = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient5(QObject *parent = nullptr);
    ~ConjugateGradient5() override;

    /** @brief Set solver parameters */
    void setParameters(double tolerance = 1e-8, int maxIterations = 1000);

    /** @brief Solve symmetric positive definite Ax = b with IC(0) PCG */
    SolveResult solve(const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /** @brief Solve non-symmetric system using CGS method */
    SolveResult solveCGS(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Build IC(0) preconditioner for matrix A */
    QVector<QVector<double>> buildIC0Preconditioner(
        const QVector<QVector<double>>& A) const;

    /** @brief Compute residual norm ||b - Ax|| */
    double residualNorm(const QVector<QVector<double>>& A,
                         const QVector<double>& x,
                         const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual);
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    double m_tolerance = 1e-8;
    int m_maxIterations = 1000;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiplication */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& x) const;

    /** @brief Dot product of two vectors */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Apply IC(0) preconditioner: solve L*L^T * z = r */
    QVector<double> applyPreconditioner(
        const QVector<QVector<double>>& L,
        const QVector<double>& r) const;

    /** @brief Forward substitution solve L*y = b */
    QVector<double> forwardSolve(const QVector<QVector<double>>& L,
                                  const QVector<double>& b) const;

    /** @brief Backward substitution solve L^T*x = y */
    QVector<double> backwardSolve(const QVector<QVector<double>>& L,
                                   const QVector<double>& y) const;
};
