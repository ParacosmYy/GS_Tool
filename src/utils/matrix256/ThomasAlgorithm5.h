/**
 * @file ThomasAlgorithm5.h
 * @brief 块三对角Thomas算法(Thomas块消元+Schur补归约) — Thomas Algorithm for Block Tridiagonal Systems with Thomas-Based Block Elimination and Schur Complement Reduction
 *
 * 功能: 实现块三对角系统求解器(block tridiagonal solver)，使用Thomas
 *       块消元(Thomas-based block elimination)前向消去和回代，Schur补
 *       归约(Schur complement reduction)处理耦合块对角元素。
 *
 * 协作: GaussianElimination3(高斯消元) / LUDecomposition4(LU分解) / IterativeSolver6(迭代求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 块三对角Thomas算法(块消元+Schur补归约)
 */
class ThomasAlgorithm5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        int blockSize = 0;
        int numBlocks = 0;
        int numSchurReductions = 0;
        double residualNorm = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm5(QObject *parent = nullptr);
    ~ThomasAlgorithm5() override;

    /** @brief Set block size for block tridiagonal system */
    void setBlockSize(int blockSize);

    /** @brief Solve scalar tridiagonal system Ax=d */
    QVector<double> solveScalar(const QVector<double>& lower,
                                 const QVector<double>& diag,
                                 const QVector<double>& upper,
                                 const QVector<double>& rhs);

    /** @brief Solve block tridiagonal system (N blocks of B x B) */
    QVector<QVector<double>> solveBlock(
        const QVector<QVector<QVector<double>>>& lowerBlocks,
        const QVector<QVector<QVector<double>>>& diagBlocks,
        const QVector<QVector<QVector<double>>>& upperBlocks,
        const QVector<QVector<double>>& rhsBlocks);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, double residual, double timeMs);

private:
    int m_blockSize = 2;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiply for block: C = A * b */
    QVector<double> matVec(const QVector<QVector<double>>& A,
                            const QVector<double>& b) const;

    /** @brief Solve small linear system A*x=b via Gaussian elimination */
    QVector<double> solveSmall(const QVector<QVector<double>>& A,
                                const QVector<double>& b) const;

    /** @brief Compute Schur complement: S = D - C * A^{-1} * B */
    QVector<QVector<double>> schurComplement(
        const QVector<QVector<double>>& D,
        const QVector<QVector<double>>& C,
        const QVector<QVector<double>>& AinvB) const;

    /** @brief Invert small matrix */
    QVector<QVector<double>> invertSmall(
        const QVector<QVector<double>>& A) const;

    /** @brief Compute residual norm ||Ax - d|| */
    double residualNorm(const QVector<double>& lower,
                         const QVector<double>& diag,
                         const QVector<double>& upper,
                         const QVector<double>& rhs,
                         const QVector<double>& x) const;
};
