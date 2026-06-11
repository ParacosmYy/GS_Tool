/**
 * @file BiCGSTAB16.h
 * @brief BiCGSTAB求解器(多项式预处理与前瞻残差监控防止不定系统停滞) — BiCGSTAB with Polynomial Preconditioning and Look-ahead Residual Monitoring for Preventing Stagnation in Indefinite Systems
 *
 * 功能: 实现BiCGSTAB求解器(BiCGSTAB solver)，采用多项式预处理(polynomial preconditioning)
 *       与前瞻残差监控(look-ahead residual monitoring)防止不定系统停滞(preventing stagnation in indefinite systems)。
 *
 * 协作: ConjugateGradient15(CG求解器) / GMRES14(GMRES) / SparseMatrix13(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

class BiCGSTAB16 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        double residualNorm = 0.0;       // Final residual
        int iterations = 0;
        bool converged = false;
        bool stagnated = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int problemSize = 0;
        double avgProcessingTimeMs = 0.0;
        double avgIterations = 0.0;
    };

    explicit BiCGSTAB16(QObject *parent = nullptr);
    ~BiCGSTAB16() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);
    void setPreconditionDegree(int deg); // Polynomial preconditioner degree

    /** @brief Solve Ax = b with sparse matrix (CSR format) */
    SolveResult solve(int n,
                       const QVector<double>& values,
                       const QVector<int>& colIdx,
                       const QVector<int>& rowPtr,
                       const QVector<double>& rhs);

    /** @brief Solve with dense matrix */
    SolveResult solveDense(const QVector<QVector<double>>& A,
                             const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int n, int iters, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    int m_precondDegree = 2;            // Neumann polynomial degree
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_iterSum = 0.0;

    /** @brief Sparse matrix-vector product y = A*x */
    void spmv(int n, const QVector<double>& val,
               const QVector<int>& col, const QVector<int>& row,
               const QVector<double>& x, QVector<double>& y) const;

    /** @brief Dense matrix-vector product */
    void denseMV(const QVector<QVector<double>>& A,
                  const QVector<double>& x, QVector<double>& y) const;

    /** @brief Polynomial preconditioner: approximate (I - A)^k */
    void polynomialPrecondition(int n,
                                  const QVector<double>& val,
                                  const QVector<int>& col,
                                  const QVector<int>& row,
                                  const QVector<double>& r,
                                  QVector<double>& z) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;
};
