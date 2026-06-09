/**
 * @file BiCGSTAB11.h
 * @brief 双共轭梯度稳定法(GPBi-CGS变体复合步残差平滑不定系统鲁棒求解) — BiCGSTAB with GPBi-CGS Variant and Composite Step with Residual Smoothing for Robust Indefinite System Solving
 *
 * 功能: 实现BiCGSTAB迭代求解器的GPBi-CGS变体，采用复合步(composite
 *       step)策略和残差平滑(residual smoothing)技术，用于鲁棒求解
 *       不定线性方程组(indefinite linear systems)。
 *
 * 协作: ConjugateGradient9(CG法) / GMRES10(GMRES) / SparseMatrix8(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 双共轭梯度稳定法(GPBi-CGS变体复合步残差平滑不定系统鲁棒求解)
 */
class BiCGSTAB11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int iterationsUsed = 0;
        double finalResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB11(QObject *parent = nullptr);
    ~BiCGSTAB11() override;

    /** @brief Set max iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Solve Ax = b where A is provided via matvec callback */
    QVector<double> solve(
        const QVector<double>& b,
        const QVector<double>& x0,
        const std::function<QVector<double>(const QVector<double>&)>& matvec);

    /** @brief Get residual history */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double residual, double timeMs);
    void solveCompleted(int iterations, double finalResidual, double timeMs);

private:
    int m_maxIter = 1000;
    double m_tol = 1e-10;

    QVector<double> m_residualHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Dot product of two vectors */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief Norm of a vector */
    static double norm(const QVector<double>& v);

    /** @brief Axpy: y = alpha*x + y */
    static void axpy(double alpha, const QVector<double>& x, QVector<double>& y);

    /** @brief Scale: y = alpha*x */
    static void scale(double alpha, QVector<double>& x);
};
