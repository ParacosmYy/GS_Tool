/**
 * @file BiCGSTAB8.h
 * @brief BiCGSTAB求解器(多项式预处理+前瞻停滞恢复不定系统) — BiCGSTAB with Polynomial Preconditioning and Look-Ahead Stagnation Recovery for Indefinite Systems
 *
 * 功能: 实现BiCGSTAB(Biconjugate Gradient Stabilized)迭代求解器，采用多项式预处理
 *       (polynomial preconditioning)加速收敛，结合前瞻停滞恢复(look-ahead stagnation
 *       recovery)策略处理不定系统(indefinite systems)的求解困难。
 *
 * 协作: ConjugateGradient9(共轭梯度) / LUDecomposition6(LU分解) / SparseMatrix7(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BiCGSTAB求解器(多项式预处理+前瞻停滞恢复不定系统)
 */
class BiCGSTAB8 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        int iterations = 0;
        double residualNorm = 0.0;
        bool converged = false;
        int stagnationRecoveries = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
        double avgResidualNorm = 0.0;
    };

    explicit BiCGSTAB8(QObject *parent = nullptr);
    ~BiCGSTAB8() override;

    /** @brief Set max iterations */
    void setMaxIterations(int iter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set preconditioning degree (0=none, 1-3=polynomial degree) */
    void setPreconditionerDegree(int degree);

    /** @brief Set stagnation detection threshold */
    void setStagnationThreshold(double threshold);

    /** @brief Solve Ax = b with dense matrix A [n x n] */
    SolveResult solve(const QVector<QVector<double>>& A, const QVector<double>& b);

    /** @brief Solve with initial guess */
    SolveResult solveWithGuess(const QVector<QVector<double>>& A,
                                const QVector<double>& b,
                                const QVector<double>& x0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual);
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    int m_precondDegree = 1;
    double m_stagnationThresh = 1e-14;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiply */
    QVector<double> matVecMul(const QVector<QVector<double>>& A,
                               const QVector<double>& x) const;

    /** @brief Polynomial preconditioner application */
    QVector<double> applyPreconditioner(const QVector<QVector<double>>& A,
                                         const QVector<double>& r) const;

    /** @brief Jacobi preconditioner diagonal */
    QVector<double> jacobiDiag(const QVector<QVector<double>>& A) const;

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norms */
    static double norm(const QVector<double>& v);

    /** @brief Scale vector */
    static QVector<double> scale(double s, const QVector<double>& v);

    /** @brief Add vectors */
    static QVector<double> add(const QVector<double>& a, const QVector<double>& b);

    /** @brief Subtract vectors */
    static QVector<double> sub(const QVector<double>& a, const QVector<double>& b);
};
