/**
 * @file ConjugateGradient3.h
 * @brief 共轭梯度法(代数多重网格AMG预条件+椭圆PDE系统) — Conjugate Gradient with Algebraic Multigrid (AMG) Preconditioner for Elliptic PDE Systems
 *
 * 功能: 实现AMG预条件共轭梯度法，支持层次网格构造、
 *       粗化/插值算子和椭圆PDE稀疏矩阵求解。
 *
 * 协作: SparseSolver7(稀疏求解) / ConjugateGradient2(CG) / GaussSeidel5(高斯-赛德尔)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 共轭梯度法(代数多重网格AMG预条件+椭圆PDE系统)
 */
class ConjugateGradient3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConjugateGradient3(QObject *parent = nullptr);
    ~ConjugateGradient3() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);
    void setAmgLevels(int levels);

    /** @brief Solve Ax = b using PCG with AMG preconditioner */
    QVector<double> solve(const QVector<QVector<double>>& A, const QVector<double>& b);

    /** @brief Build AMG hierarchy from matrix A */
    void buildAMG(const QVector<QVector<double>>& A);

    /** @brief Apply AMG V-cycle as preconditioner: solve Mz = r */
    QVector<double> amgVCycle(const QVector<double>& residual, int level) const;

    /** @brief Coarsening: select coarse grid points via strong connections */
    QVector<int> coarsen(const QVector<QVector<double>>& mat) const;

    /** @brief Build interpolation operator P (fine -> coarse) */
    QVector<QVector<double>> buildInterpolation(const QVector<QVector<double>>& mat,
                                                  const QVector<int>& coarseSet) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, int iterations, double residual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-8;
    int m_amgLevels = 3;

    // AMG hierarchy storage
    QVector<QVector<QVector<double>>> m_Alevels; // matrices per level
    QVector<QVector<QVector<double>>> m_Plevels; // interpolation operators
    QVector<QVector<QVector<double>>> m_Rlevels; // restriction operators

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector product */
    static QVector<double> matVec(const QVector<QVector<double>>& A, const QVector<double>& x);

    /** @brief Dot product */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Gauss-Seidel smoother (one sweep) */
    static void gaussSeidel(const QVector<QVector<double>>& A,
                              QVector<double>& x, const QVector<double>& b, int sweeps);

    /** @brief Restriction: R = P^T (transpose of interpolation) */
    static QVector<QVector<double>> transpose(const QVector<QVector<double>>& P);
};
