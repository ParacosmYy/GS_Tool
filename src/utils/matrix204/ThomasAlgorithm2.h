/**
 * @file ThomasAlgorithm2.h
 * @brief 三对角求解器(SPIKE算法+分布式内存分区系统) — Tridiagonal Solver with SPIKE Algorithm for Distributed-Memory Partitioned Systems
 *
 * 功能: 实现三对角矩阵求解器，支持SPIKE分布式分区算法、
 *       Thomas消元和多分区并行消元回代。
 *
 * 协作: LUDecomposition5(LU分解) / GaussElimination4(高斯消元) / SparseSolver3(稀疏求解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角求解器(SPIKE算法+分布式内存分区系统)
 */
class ThomasAlgorithm2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSolves = 0;
        int systemSize = 0;
        int numPartitions = 1;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ThomasAlgorithm2(QObject *parent = nullptr);
    ~ThomasAlgorithm2() override;

    void setNumPartitions(int p);

    /** @brief Classic Thomas algorithm for tridiagonal Ax=d */
    QVector<double> thomasSolve(const QVector<double>& lower,
                                 const QVector<double>& diag,
                                 const QVector<double>& upper,
                                 const QVector<double>& rhs) const;

    /** @brief SPIKE algorithm: partitioned tridiagonal solve */
    QVector<double> spikeSolve(const QVector<double>& lower,
                                const QVector<double>& diag,
                                const QVector<double>& upper,
                                const QVector<double>& rhs);

    /** @brief Solve a single partition via Thomas */
    QVector<double> solvePartition(const QVector<double>& l,
                                    const QVector<double>& d,
                                    const QVector<double>& u,
                                    const QVector<double>& r) const;

    /** @brief Build reduced SPIKE tip system */
    void buildSpikeTips(const QVector<double>& lower,
                         const QVector<double>& diag,
                         const QVector<double>& upper,
                         int partSize,
                         QVector<double>& tipV, QVector<double>& tipW) const;

    /** @brief Validate tridiagonal system */
    static bool validate(const QVector<double>& lower,
                          const QVector<double>& diag,
                          const QVector<double>& upper);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int size, int partitions, double timeMs);

private:
    int m_numPartitions = 1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Thomas forward elimination */
    void forwardEliminate(QVector<double>& d, QVector<double>& u,
                           QVector<double>& r) const;

    /** @brief Thomas backward substitution */
    QVector<double> backwardSubstitute(const QVector<double>& d,
                                        const QVector<double>& u,
                                        const QVector<double>& r) const;
};
