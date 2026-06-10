/**
 * @file TridiagonalSolver7.h
 * @brief 三对角求解器(SPIKE分区与并行子域消元块三对角系统) — Tridiagonal Solver with SPIKE Partitioning and Parallel Subdomain Elimination for Block-Tridiagonal Systems
 *
 * 功能: 实现三对角求解器(Tridiagonal solver)，采用SPIKE分区(SPIKE partitioning)
 *       与并行子域消元(parallel subdomain elimination)实现块三对角系统求解(block-tridiagonal systems)。
 *
 * 协作: ThomasAlgorithm3(Thomas算法) / LU8(LU分解) / GaussSeidel7(高斯-赛德尔迭代)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角求解器(SPIKE分区与并行子域消元块三对角系统)
 */
class TridiagonalSolver7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int systemSize = 0;
        int numPartitions = 1;
        double residual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalSolver7(QObject *parent = nullptr);
    ~TridiagonalSolver7() override;

    /** @brief Set number of partitions for SPIKE algorithm */
    void setPartitions(int p);

    /** @brief Solve tridiagonal system Ax=d using Thomas algorithm */
    QVector<double> solveThomas(const QVector<double>& lower,
                                const QVector<double>& diag,
                                const QVector<double>& upper,
                                const QVector<double>& rhs);

    /** @brief Solve using SPIKE partitioning for block-tridiagonal systems */
    QVector<double> solveSpike(const QVector<double>& lower,
                               const QVector<double>& diag,
                               const QVector<double>& upper,
                               const QVector<double>& rhs);

    /** @brief Compute residual ||Ax - d|| / ||d|| */
    double computeResidual(const QVector<double>& lower,
                           const QVector<double>& diag,
                           const QVector<double>& upper,
                           const QVector<double>& rhs,
                           const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, int partitions, double residual, double timeMs);

private:
    int m_partitions = 4;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Solve a single partition subsystem via Thomas */
    QVector<double> solvePartition(const QVector<double>& a,
                                   const QVector<double>& b,
                                   const QVector<double>& c,
                                   const QVector<double>& d,
                                   int start, int end) const;

    /** @brief Compute reduced spike system coupling equations */
    QVector<QVector<double>> buildSpikeSystem(
        const QVector<QVector<double>>& partitionTips) const;
};
