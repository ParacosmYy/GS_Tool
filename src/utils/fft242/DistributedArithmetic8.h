/**
 * @file DistributedArithmetic8.h
 * @brief 分布式算术(查找表分区+流水线累加定点FIR求值) — Distributed Arithmetic with Lookup Table Partitioning and Pipelined Accumulator for Fixed-Point FIR Evaluation
 *
 * 功能: 实现分布式算术(Distributed arithmetic)FIR滤波器求值，采用查找表分区
 *       (lookup table partitioning)将滤波器系数编码为预计算查分表，通过流水线
 *       累加器(pipelined accumulator)逐位累加实现无乘法器定点FIR卷积运算。
 *
 * 协作: WHT7(沃尔什-哈达玛变换) / BruunFFT8(Bruun FFT) / DST8(离散正弦变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术(查找表分区+流水线累加定点FIR求值)
 */
class DistributedArithmetic8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterLength = 0;
        int lutEntries = 0;
        int numPartitions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DistributedArithmetic8(QObject *parent = nullptr);
    ~DistributedArithmetic8() override;

    /** @brief Set FIR filter coefficients (fixed-point scaled) */
    bool setCoefficients(const QVector<double>& coeffs);

    /** @brief Set fractional bits for fixed-point representation */
    void setFractionalBits(int bits);

    /** @brief Process input samples through DA-FIR */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process single sample */
    double processOne(double sample);

    /** @brief Reset internal shift registers */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterConfigured(int length, int lutSize);
    void processingCompleted(int numSamples, double timeMs);

private:
    int m_fracBits = 16;
    int m_taps = 0;
    int m_partitions = 0;

    QVector<double> m_coeffs;
    QVector<QVector<qint32>> m_lut;      // LUT per partition
    QVector<qint32> m_shiftReg;           // Shift register holding recent inputs

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build LUT: enumerate all input combinations for a partition */
    void buildLUT();

    /** @brief Convert double to fixed-point */
    qint32 toFixed(double val) const;

    /** @brief Convert fixed-point back to double */
    double fromFixed(qint32 val) const;

    /** @brief Number of taps per LUT partition */
    static constexpr int kPartitionSize = 4;
};
