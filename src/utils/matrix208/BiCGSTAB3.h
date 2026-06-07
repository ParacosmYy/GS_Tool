/**
 * @file BiCGSTAB3.h
 * @brief 稳定双共轭梯度法(GPBi-CG稳定化+嵌套Schur补预条件) — BiCGSTAB with GPBi-CG Stabilization for Irregular Convergence and Nested Schur Complement Preconditioner
 *
 * 功能: 实现稳定化BiCGSTAB线性方程组求解器，支持GPBi-CG稳定化、
 *       嵌套Schur补预条件和收敛监测。
 *
 * 协作: CGSolver6(共轭梯度) / SparseMatrix7(稀疏矩阵) / GMRES5(GMRES)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 稳定双共轭梯度法(GPBi-CG稳定化+嵌套Schur补预条件)
 */
class BiCGSTAB3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB3(QObject *parent = nullptr);
    ~BiCGSTAB3() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setPreconditionerLevels(int levels);

    /** @brief Solve Ax=b with sparse matrix (CSR format) */
    QVector<double> solve(const QVector<double>& values,
                           const QVector<int>& colIndices,
                           const QVector<int>& rowPtr,
                           const QVector<double>& rhs,
                           int n);

    /** @brief Apply nested Schur complement preconditioner */
    QVector<double> applyPreconditioner(const QVector<double>& r,
                                         const QVector<double>& values,
                                         const QVector<int>& colIndices,
                                         const QVector<int>& rowPtr,
                                         int n) const;

    /** @brief Compute residual norm ||b - Ax|| */
    double residualNorm(const QVector<double>& x,
                         const QVector<double>& values,
                         const QVector<int>& colIndices,
                         const QVector<int>& rowPtr,
                         const QVector<double>& rhs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);
    void iterationUpdate(int iter, double residual);

private:
    int m_maxIter = 1000;
    double m_tolerance = 1e-10;
    int m_precLevels = 2;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector product y = A*x */
    static QVector<double> spmv(const QVector<double>& values,
                                 const QVector<int>& colIndices,
                                 const QVector<int>& rowPtr,
                                 const QVector<double>& x, int n);

    /** @brief Dot product of two vectors */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Build diagonal Jacobi preconditioner */
    QVector<double> buildJacobi(const QVector<double>& values,
                                 const QVector<int>& rowPtr, int n) const;

    /** @brief Perform ILU(0) factorization for Schur complement */
    void ilu0Factorize(const QVector<double>& values,
                        const QVector<int>& colIndices,
                        const QVector<int>& rowPtr,
                        QVector<double>& luValues, int n) const;
};
