/**
 * @file GaussSeidel2.h
 * @brief 多重网格Gauss-Seidel(V循环粗网格校正+限制/延拓算子) — Multigrid Gauss-Seidel with V-Cycle Coarse Grid Correction and Restriction/Prolongation Operators
 *
 * 功能: 实现多重网格Gauss-Seidel求解器，支持V循环、
 *       限制算子、延拓算子和粗网格校正。
 *
 * 协作: ConjugateGradient4(共轭梯度) / SOR3(SOR迭代) / Cholesky6(Cholesky分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多重网格Gauss-Seidel(V循环粗网格校正+限制/延拓算子)
 */
class GaussSeidel2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int gridSize = 0;
        int iterations = 0;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussSeidel2(QObject *parent = nullptr);
    ~GaussSeidel2() override;

    void setMaxIterations(int iters);
    void setTolerance(double tol);
    void setOmega(double omega);
    void setMaxLevels(int levels);

    /** @brief Solve Ax = b using multigrid Gauss-Seidel */
    QVector<double> solve(const QVector<QVector<double>>& A, const QVector<double>& b);

    /** @brief Single Gauss-Seidel sweep (SOR) */
    void gsSweep(QVector<double>& x, const QVector<QVector<double>>& A,
                  const QVector<double>& b) const;

    /** @brief Compute residual r = b - Ax */
    QVector<double> residual(const QVector<QVector<double>>& A,
                              const QVector<double>& x,
                              const QVector<double>& b) const;

    /** @brief Restrict fine grid residual to coarse grid */
    QVector<double> restrict_(const QVector<double>& fine) const;

    /** @brief Prolongate coarse grid correction to fine grid */
    QVector<double> prolongate(const QVector<double>& coarse, int fineSize) const;

    /** @brief V-cycle multigrid correction */
    QVector<double> vCycle(QVector<double> x, const QVector<QVector<double>>& A,
                            const QVector<double>& b, int level) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    int m_maxIterations = 100;
    double m_tolerance = 1e-8;
    double m_omega = 1.0;   // SOR relaxation (1.0 = pure GS)
    int m_maxLevels = 5;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute L2 norm of vector */
    static double l2Norm(const QVector<double>& v);
};
