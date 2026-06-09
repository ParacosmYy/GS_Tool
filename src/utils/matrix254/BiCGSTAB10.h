/**
 * @file BiCGSTAB10.h
 * @brief 稳定双共轭梯度法(GPBi-CAB复合步变体+残差平滑单调收敛保证) — BiCGSTAB with GPBi-CAB Composite Step Variant and Residual Smoothing for Monotonic Convergence Guarantee
 *
 * 功能: 实现BiCGSTAB求解器(BiConjugate Gradient Stabilized method)，使用
 *       GPBi-CAB复合步变体(GPBi-CAB composite step)处理不规则收敛行为，
 *       残差平滑(residual smoothing)确保单调收敛保证。
 *
 * 协作: ConjugateGradient9(共轭梯度) / GMRES8(GMRES) / SparseLU7(稀疏LU)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BiCGSTAB求解器(GPBi-CAB复合步+残差平滑)
 */
class BiCGSTAB10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        int maxIterations = 0;
        double initialResidual = 0.0;
        double finalResidual = 0.0;
        double tolerance = 0.0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB10(QObject *parent = nullptr);
    ~BiCGSTAB10() override;

    /** @brief Set convergence tolerance and max iterations */
    void setTolerance(double tol, int maxIter);

    /** @brief Solve Ax = b where A is sparse (CSR format) */
    QVector<double> solve(const QVector<double>& values,
                           const QVector<int>& colIdx,
                           const QVector<int>& rowPtr,
                           const QVector<double>& rhs);

    /** @brief Get residual history from last solve */
    QVector<double> residualHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int iterations, double residual, double timeMs);

private:
    double m_tolerance = 1e-8;
    int m_maxIter = 1000;

    QVector<double> m_residualHistory;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector multiply y = A*x (CSR) */
    static void spmv(const QVector<double>& val,
                      const QVector<int>& col,
                      const QVector<int>& rowPtr,
                      const QVector<double>& x,
                      QVector<double>& y);

    /** @brief Dot product */
    static double dot(const QVector<double>& a,
                       const QVector<double>& b);

    /** @brief Axpy: y = alpha*x + y */
    static void axpy(double alpha, const QVector<double>& x,
                      QVector<double>& y);

    /** @brief Vector norm */
    static double norm(const QVector<double>& v);

    /** @brief GPBi-CAB composite step: combined product computation */
    void compositeStep(const QVector<double>& val,
                        const QVector<int>& col,
                        const QVector<int>& rowPtr,
                        const QVector<double>& r,
                        const QVector<double>& p,
                        QVector<double>& Ap,
                        QVector<double>& Ar) const;

    /** @brief Apply residual smoothing to guarantee monotonic decrease */
    void smoothResidual(QVector<double>& rSmooth,
                         const QVector<double>& rNew,
                         double& rhoSmooth) const;
};
