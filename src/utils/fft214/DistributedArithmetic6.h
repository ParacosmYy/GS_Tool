/**
 * @file DistributedArithmetic6.h
 * @brief 分布式算术(LUT分区+流水线累加FIR滤波加速) — Distributed Arithmetic with LUT Partitioning and Pipelined Accumulator for FIR Filter Acceleration
 *
 * 功能: 实现分布式算术FIR滤波，支持LUT分区查找、
 *       流水线位串行累加和可配置滤波器阶数。
 *
 * 协作: FIRFilter2(FIR滤波器) / CICFilter3(CIC滤波器) / WHT5(沃尔什-哈达玛变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术(LUT分区+流水线累加FIR)
 */
class DistributedArithmetic6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalSamples = 0;
        int filterOrder = 0;
        int lutEntries = 0;
        int partitions = 0;
        int bitWidth = 16;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DistributedArithmetic6(QObject *parent = nullptr);
    ~DistributedArithmetic6() override;

    /** @brief Initialize with FIR coefficients */
    void setCoefficients(const QVector<double>& coeffs, int bitWidth = 16);

    /** @brief Process single sample */
    double processOne(double input);

    /** @brief Process buffer */
    QVector<double> process(const QVector<double>& input);

    /** @brief Query LUT for a given address and partition */
    double queryLUT(int partition, int address) const;

    /** @brief Get current coefficients */
    QVector<double> coefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double timeMs);

private:
    QVector<double> m_coeffs;
    int m_order = 0;
    int m_bitWidth = 16;
    int m_partitions = 1;
    int m_lutSize = 0;

    // Partitioned LUTs
    QVector<QVector<double>> m_luts;
    // Coefficients per partition
    int m_coeffsPerPart = 0;

    // Input shift register (for direct form fallback)
    QVector<double> m_shiftReg;
    int m_regPos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build partitioned LUTs from coefficients */
    void buildLUTs();

    /** @brief Convert double sample to fixed-point integer */
    qint32 toFixed(double val) const;

    /** @brief Convert fixed-point back to double */
    double fromFixed(qint64 val) const;
};
