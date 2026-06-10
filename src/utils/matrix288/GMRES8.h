/**
 * @file GMRES8.h
 * @brief GMRES求解器(泄气重启与调和Ritz值的内部特征值增强子空间回收) — GMRES with Deflated Restarting and Harmonic Ritz Values for Interior Eigenvalue-augmented Subspace Recycling
 *
 * 功能: 实现GMRES求解器(GMRES solver)，采用泄气重启(deflated restarting)
 *       与调和Ritz值(harmonic Ritz values)实现内部特征值增强子空间回收(interior eigenvalue-augmented subspace recycling)。
 *
 * 协作: BiCGSTAB7(BiCGSTAB) / SparseMatrix6(稀疏矩阵) / EigenSolver5(特征值求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES求解器(泄气重启与调和Ritz值)
 */
class GMRES8 : public QObject {
    Q_OBJECT

public:
    /** @brief Solver configuration */
    struct GMRESConfig {
        int maxIterations = 1000;
        int restartLength = 30;
        int deflationSize = 5;
        double tolerance = 1e-10;
    };

    /** @brief Solver result */
    struct GMRESResult {
        QVector<double> x;
        double residual = 0.0;
        int iterations = 0;
        int restarts = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int problemSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES8(QObject *parent = nullptr);
    ~GMRES8() override;

    void setConfig(const GMRESConfig& cfg);

    /** @brief Solve Ax=b where A is given as a matrix */
    GMRESResult solve(const QVector<QVector<double>>& A,
                      const QVector<double>& b);

    /** @brief Solve using callback (matrix-free) */
    using MatVecFn = std::function<QVector<double>(const QVector<double>&)>;
    GMRESResult solveMatrixFree(MatVecFn Av, const QVector<double>& b, int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int iters, double residual, double timeMs);

private:
    GMRESConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Deflation subspace: cached eigenvectors from previous restarts
    QVector<QVector<double>> m_deflVectors;
    int m_deflCount = 0;

    /** @brief Arnoldi process: build orthonormal basis V and upper Hessenberg H */
    int arnoldi(int n, const QVector<double>& q,
                QVector<QVector<double>>& V, QVector<double>& H,
                int startCol, int maxCols, MatVecFn Av);

    /** @brief Solve least-squares problem for y in min||beta*e1 - H*y|| */
    QVector<double> leastSquares(const QVector<double>& H, int m, int n,
                                  double beta) const;

    /** @brief Compute harmonic Ritz values for deflation */
    QVector<double> harmonicRitz(const QVector<QVector<double>>& V,
                                  const QVector<double>& H, int m) const;

    /** @brief Update deflation subspace with selected eigenvectors */
    void updateDeflation(const QVector<QVector<double>>& V,
                          const QVector<double>& H, int m);

    /** @brief Apply deflation: project out converged eigenvectors */
    QVector<double> applyDeflation(const QVector<double>& v) const;

    /** @brief Dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector norm */
    double norm(const QVector<double>& v) const;
};
