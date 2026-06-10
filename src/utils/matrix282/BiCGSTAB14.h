/**
 * @file BiCGSTAB14.h
 * @brief 双共轭梯度稳定法(GPBi-CGS稳定变体与复合残差步的病态系统保证收敛求解器) — BiCGSTAB with GPBi-CGS Stabilized Variant and Composite Residual Step for Guaranteed Convergence in Ill-conditioned Systems
 *
 * 功能: 实现双共轭梯度稳定法(BiCGSTAB)，采用GPBi-CGS稳定变体(GPBi-CGS stabilized variant)
 *       与复合残差步(composite residual step)实现病态系统保证收敛求解器(guaranteed convergence in ill-conditioned systems)。
 *
 * 协作: ConjugateGradient12(共轭梯度) / GMRES13(GMRES) / SparseMatrix8(稀疏矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 双共轭梯度稳定法(GPBi-CGS稳定变体与复合残差步)
 */
class BiCGSTAB14 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver result */
    struct SolveResult {
        QVector<double> x;            // Solution vector
        double initialResidual = 0.0;
        double finalResidual = 0.0;
        int iterations = 0;
        bool converged = false;
        double relativeResidual = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiCGSTAB14(QObject *parent = nullptr);
    ~BiCGSTAB14() override;

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set maximum iterations */
    void setMaxIterations(int iter);

    /** @brief Set composite residual step frequency (0 = disabled) */
    void setCompositeStepFrequency(int freq);

    /** @brief Solve Ax = b with sparse matrix (COO format) */
    SolveResult solve(int n, const QVector<int>& rowIdx, const QVector<int>& colIdx,
                       const QVector<double>& values, const QVector<double>& b);

    /** @brief Solve with dense matrix */
    SolveResult solveDense(const QVector<QVector<double>>& A, const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationUpdate(int iter, double residual);
    void solveDone(int iterations, double finalResidual, bool converged, double timeMs);

private:
    double m_tolerance = 1e-8;
    int m_maxIter = 1000;
    int m_compStepFreq = 5;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Sparse matrix-vector multiply: y = A * x */
    void spmv(int n, const QVector<int>& rowIdx, const QVector<int>& colIdx,
              const QVector<double>& values, const QVector<double>& x,
              QVector<double>& y) const;

    /** @brief Dense matrix-vector multiply */
    void denseMV(const QVector<QVector<double>>& A, const QVector<double>& x,
                  QVector<double>& y) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;
};
