/**
 * @file ConjugateGradient2.h
 * @brief 预条件共轭梯度(IC(0)不完全Cholesky+柔性变体FCG) — Preconditioned Conjugate Gradient with Incomplete Cholesky IC(0) Preconditioner and Flexible Variant (FCG)
 *
 * 功能: 实现预条件共轭梯度法，支持IC(0)不完全Cholesky预条件、
 *       柔性共轭梯度(FCG)变体和收敛监控。
 *
 * 协作: SparseMatrix5(稀疏矩阵) / Cholesky3(Cholesky分解) / GMRES4(GMRES)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 预条件共轭梯度(IC(0)+FCG)
 */
class ConjugateGradient2 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver mode */
    enum Mode { StandardPCG, FlexibleFCG };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterations = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient2(QObject *parent = nullptr);
    ~ConjugateGradient2() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setMode(Mode mode);

    /** @brief Solve Ax = b with sparse SPD matrix */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief Solve with triplet format (row, col, val) sparse matrix */
    QVector<double> solveSparse(const QVector<int>& rows,
                                const QVector<int>& cols,
                                const QVector<double>& vals,
                                int n,
                                const QVector<double>& b);

    /** @brief Compute IC(0) incomplete Cholesky preconditioner */
    void buildPreconditioner(const QVector<QVector<double>>& A);

    /** @brief Get convergence history (residual per iteration) */
    QVector<double> convergenceHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tolerance = 1e-8;
    Mode m_mode = StandardPCG;

    QVector<QVector<double>> m_L;  // IC(0) lower triangular
    QVector<double> m_diag;        // diagonal of L

    QVector<double> m_residuals;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Forward solve L*y = b */
    QVector<double> forwardSolve(const QVector<double>& b) const;

    /** @brief Backward solve L^T*x = y */
    QVector<double> backwardSolve(const QVector<double>& y) const;

    /** @brief Apply preconditioner: M^{-1} * r */
    QVector<double> applyPreconditioner(const QVector<double>& r) const;

    /** @brief Sparse matrix-vector product */
    static QVector<double> spmv(const QVector<QVector<double>>& A,
                                const QVector<double>& x);

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Compute residual ||b - Ax|| */
    static double computeResidual(const QVector<QVector<double>>& A,
                                  const QVector<double>& x,
                                  const QVector<double>& b);
};
