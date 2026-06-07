/**
 * @file BiCGSTAB2.h
 * @brief BiCGSTAB求解器(GPBi-CG变体+多项式预处理) — BiCGSTAB with GPBi-CG Variant for Improved Convergence and Polynomial Preconditioning
 *
 * 功能: 实现BiCGSTAB线性方程组求解器，支持GPBi-CG变体、
 *       改进收敛性和多项式预处理。
 *
 * 协作: ConjugateGradient8(CG求解器) / GMRES7(GMRES) / SparseMatrix6(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BiCGSTAB求解器(GPBi-CG变体+多项式预处理)
 */
class BiCGSTAB2 : public QObject {
    Q_OBJECT

public:
    /** @brief Sparse matrix entry */
    struct SparseEntry {
        int row = 0;
        int col = 0;
        double value = 0.0;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterations = 0;
        double finalResidual = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB2(QObject *parent = nullptr);
    ~BiCGSTAB2() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setPreconditionDegree(int deg);

    /** @brief Solve Ax = b using BiCGSTAB with GPBi-CG variant */
    QVector<double> solve(int n, const QVector<SparseEntry>& entries,
                          const QVector<double>& b);

    /** @brief Solve with dense matrix (converts to sparse) */
    QVector<double> solveDense(const QVector<QVector<double>>& A,
                               const QVector<double>& b);

    /** @brief Apply polynomial preconditioner */
    QVector<double> precondition(const QVector<double>& r) const;

    /** @brief Compute residual norm */
    double residualNorm(const QVector<double>& r) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, bool converged, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tolerance = 1e-10;
    int m_precondDeg = 2;

    // CSR sparse matrix storage
    int m_n = 0;
    QVector<double> m_values;
    QVector<int> m_colIdx;
    QVector<int> m_rowPtr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build CSR from sparse entries */
    void buildCSR(int n, const QVector<SparseEntry>& entries);

    /** @brief Sparse matrix-vector multiply */
    QVector<double> spmv(const QVector<double>& x) const;

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Compute polynomial preconditioner coefficients */
    QVector<double> precondCoeffs() const;
};
