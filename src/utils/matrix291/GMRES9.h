/**
 * @file GMRES9.h
 * @brief GMRES求解器(缩减重启与调和Ritz向量回收求解移位线性系统序列) — GMRES with Deflated Restarting and Harmonic Ritz Vector Recycling for Solving Sequences of Shifted Linear Systems
 *
 * 功能: 实现GMRES求解器，采用缩减重启(deflated restarting)
 *       与调和Ritz向量回收(harmonic Ritz vector recycling)实现移位线性系统序列求解(solving sequences of shifted linear systems)。
 *
 * 协作: BiCGSTAB8(BiCGSTAB) / SparseLU7(稀疏LU) / QR6(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief GMRES求解器(缩减重启与调和Ritz向量回收求解移位线性系统序列)
 */
class GMRES9 : public QObject {
    Q_OBJECT

public:
    /** @brief Matrix-vector product callback type */
    using MatVecFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief Solver configuration */
    struct GMRESConfig {
        int maxRestart = 30;            // Restart cycle length (m)
        int maxIterations = 1000;
        double tolerance = 1e-10;
        int numRecycled = 3;            // Harmonic Ritz vectors to recycle
    };

    /** @brief Solver result */
    struct SolveResult {
        QVector<double> solution;
        double residualNorm = 0.0;
        double initialResidual = 0.0;
        int iterations = 0;
        int restarts = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GMRES9(QObject *parent = nullptr);
    ~GMRES9() override;

    void setConfig(const GMRESConfig& cfg);

    /** @brief Solve Ax = b with matrix-vector product callback */
    SolveResult solve(MatVecFunc matVec, const QVector<double>& b,
                      const QVector<double>& x0 = {});

    /** @brief Solve sequence of shifted systems (A + sigma*I)x = b */
    QVector<SolveResult> solveShiftedSequence(
        MatVecFunc matVec, const QVector<double>& b,
        const QVector<double>& shifts,
        const QVector<double>& x0 = {});

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveDone(int iters, double residual, double timeMs);

private:
    GMRESConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Recycled subspace from previous solve
    QVector<QVector<double>> m_recycledBasis;

    /** @brief Arnoldi process: build orthonormal basis */
    int arnoldi(MatVecFunc& matVec, const QVector<QVector<double>>& V,
                QVector<double>& h, int j, int m);

    /** @brief Apply Givens rotations to upper Hessenberg matrix */
    void applyGivens(QVector<QVector<double>>& H, QVector<double>& cs,
                     QVector<double>& sn, int j);

    /** @brief Solve least-squares problem for y */
    QVector<double> solveLeastSquares(const QVector<QVector<double>>& H,
                                       const QVector<double>& g, int k) const;

    /** @brief Compute harmonic Ritz vectors for recycling */
    QVector<QVector<double>> computeHarmonicRitz(
        const QVector<QVector<double>>& V,
        const QVector<QVector<double>>& H, int k) const;

    /** @brief Orthonormalize against existing basis using modified Gram-Schmidt */
    void orthonormalize(QVector<double>& v,
                        const QVector<QVector<double>>& basis) const;

    /** @brief Euclidean norm */
    static double vecNorm(const QVector<double>& v);

    /** @brief Dot product */
    static double dotProduct(const QVector<double>& a, const QVector<double>& b);
};
