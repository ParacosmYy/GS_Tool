/**
 * @file ConjugateGradient10.h
 * @brief 共轭梯度法(预处理多项式与自适应步长控制实现对称正定系统求解) — Conjugate Gradient with Preconditioned Polynomial and Adaptive Step-length Control for Symmetric Positive Definite Systems
 *
 * 功能: 实现共轭梯度法(Conjugate Gradient method)，采用预处理多项式(preconditioned polynomial)
 *       与自适应步长控制(adaptive step-length control)实现对称正定系统求解(SPD system solving)。
 *
 * 协作: LUDecomposition10(LU分解) / QRDecomposition10(QR分解) / Cholesky10(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>

class ConjugateGradient10 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> x;             // Solution vector
        int iterations = 0;
        double residualNorm = 0.0;     // Final residual norm
        bool converged = false;
    };

    /** @brief Preconditioner type */
    enum Preconditioner {
        None = 0,
        Jacobi = 1,
        SSOR = 2,
        IC0 = 3       // Incomplete Cholesky (zero fill)
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient10(QObject *parent = nullptr);
    ~ConjugateGradient10() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);
    void setPreconditioner(Preconditioner prec);
    void setSSORParameter(double omega);

    /** @brief Solve Ax = b where A is SPD */
    SolveResult solve(const QVector<QVector<double>>& A,
                       const QVector<double>& b);

    /** @brief Solve with sparse matrix (COO format) */
    SolveResult solveSparse(int n,
                             const QVector<int>& rows,
                             const QVector<int>& cols,
                             const QVector<double>& vals,
                             const QVector<double>& b);

    /** @brief Build Jacobi preconditioner (diagonal inverse) */
    QVector<double> buildJacobiPrecond(const QVector<QVector<double>>& A) const;

    /** @brief Compute residual r = b - Ax */
    QVector<double> computeResidual(const QVector<QVector<double>>& A,
                                      const QVector<double>& x,
                                      const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, int iters, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    Preconditioner m_prec = None;
    double m_omega = 1.2;              // SSOR relaxation parameter
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiply y = Ax */
    QVector<double> matVecMul(const QVector<QVector<double>>& A,
                                const QVector<double>& x) const;

    /** @brief Dot product */
    double dotProduct(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Apply preconditioner: solve Mz = r */
    QVector<double> applyPreconditioner(const QVector<double>& r,
                                          const QVector<QVector<double>>& A) const;

    /** @brief Adaptive step-length computation */
    double adaptiveStepLength(const QVector<double>& p,
                                const QVector<double>& Ap,
                                double rr) const;

    /** @brief Build SSOR preconditioner matrix (simplified) */
    QVector<QVector<double>> buildSSORPrecond(
        const QVector<QVector<double>>& A) const;

    /** @brief Build incomplete Cholesky factor */
    QVector<QVector<double>> buildIC0Precond(
        const QVector<QVector<double>>& A) const;

    /** @brief Forward substitution Lx = b */
    QVector<double> forwardSolve(const QVector<QVector<double>>& L,
                                   const QVector<double>& b) const;

    /** @brief Back substitution Ux = b */
    QVector<double> backSolve(const QVector<QVector<double>>& U,
                                const QVector<double>& b) const;
};
