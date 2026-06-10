/**
 * @file BiCGSTAB12.h
 * @brief BiCGSTAB求解器(前瞻Lanczos与防崩溃多项式更新鲁棒非对称线性系统求解) — BiCGSTAB with Look-ahead Lanczos and Breakdown-proof Polynomial Update for Robust Unsymmetric Linear System Solving
 *
 * 功能: 实现BiCGSTAB求解器(BiCGSTAB solver)，采用前瞻Lanczos(look-ahead Lanczos)
 *       和防崩溃多项式更新(breakdown-proof polynomial update)实现鲁棒非对称线性系统求解
 *       (robust unsymmetric linear system solving)。
 *
 * 协作: ConjugateGradient11(共轭梯度) / GaussSeidel10(Gauss-Seidel) / GMRES10(GMRES)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BiCGSTAB求解器(前瞻Lanczos与防崩溃多项式更新鲁棒非对称线性系统求解)
 */
class BiCGSTAB12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double initialResidual = 0.0;
        double finalResidual = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB12(QObject *parent = nullptr);
    ~BiCGSTAB12() override;

    /** @brief Set solver parameters */
    void setParameters(int maxIterations = 1000, double tolerance = 1e-10);

    /** @brief Solve Ax = b where A is stored as CSR-like sparse rows */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b,
                          const QVector<double>& x0 = QVector<double>());

    /** @brief Solve with matrix-vector product callback (matrix-free) */
    QVector<double> solveWithPrecond(
        const QVector<QVector<double>>& A,
        const QVector<double>& b,
        const QVector<double>& x0 = QVector<double>());

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveUpdated(int iter, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-10;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiply */
    static QVector<double> matVec(const QVector<QVector<double>>& A,
                                  const QVector<double>& x);

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);

    /** @brief Scale vector */
    static QVector<double> scale(double s, const QVector<double>& v);

    /** @brief Vector add: a + s*b */
    static QVector<double> addScaled(const QVector<double>& a, double s,
                                      const QVector<double>& b);

    /** @brief Jacobi preconditioner (diagonal inverse) */
    static QVector<double> jacobiPrecond(const QVector<QVector<double>>& A,
                                          const QVector<double>& r);
};
