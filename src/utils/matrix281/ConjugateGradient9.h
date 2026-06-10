/**
 * @file ConjugateGradient9.h
 * @brief 共轭梯度法(Polak-Ribiere公式与自动谱界估计的加速收敛稀疏线性方程组求解) — Conjugate Gradient with Polak-Ribiere Formula and Automatic Spectral Bounds Estimation for Accelerated Convergence in Sparse Linear System Solving
 *
 * 功能: 实现共轭梯度法(Conjugate Gradient method)，采用Polak-Ribiere公式(Polak-Ribiere formula)
 *       与自动谱界估计(automatic spectral bounds estimation)实现加速收敛稀疏线性方程组求解(accelerated convergence sparse linear system solving)。
 *
 * 协作: SparseMatrix8(稀疏矩阵) / Preconditioner7(预条件) / LeastSquares10(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度法(Polak-Ribiere公式与自动谱界估计)
 */
class ConjugateGradient9 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> x;           // Solution vector
        double residualNorm = 0.0;   // Final residual norm
        int iterations = 0;          // Iterations performed
        bool converged = false;
        double spectralMin = 0.0;    // Estimated min eigenvalue
        double spectralMax = 0.0;    // Estimated max eigenvalue
        double conditionNumber = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int totalIterations = 0;
        int numConverged = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient9(QObject *parent = nullptr);
    ~ConjugateGradient9() override;

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set max iterations */
    void setMaxIterations(int iter);

    /** @brief Solve Ax = b with sparse matrix A (compressed rows) */
    SolveResult solve(const QVector<QVector<QPair<int, double>>>& A,
                       const QVector<double>& b);

    /** @brief Solve with dense matrix A */
    SolveResult solveDense(const QVector<QVector<double>>& A,
                            const QVector<double>& b);

    /** @brief Estimate spectral bounds of A */
    QPair<double, double> estimateSpectralBounds(
        const QVector<QVector<QPair<int, double>>>& A) const;

    /** @brief Compute residual r = b - Ax */
    QVector<double> residual(const QVector<QVector<double>>& A,
                              const QVector<double>& x,
                              const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationUpdate(int iter, double residual, double spectralEst);
    void solveComplete(int n, int iterations, double residual, double timeMs);

private:
    double m_tolerance = 1e-8;
    int m_maxIter = 1000;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector multiply: y = A*x */
    static void spmv(const QVector<QVector<QPair<int, double>>>& A,
                      const QVector<double>& x, QVector<double>& y);

    /** @brief Dense matrix-vector multiply */
    static void densemv(const QVector<QVector<double>>& A,
                         const QVector<double>& x, QVector<double>& y);

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector operations: y = a*x + b*y */
    static void axpy(double alpha, const QVector<double>& x,
                      double beta, QVector<double>& y);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);
};
