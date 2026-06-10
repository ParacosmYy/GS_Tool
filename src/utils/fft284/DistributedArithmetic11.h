/**
 * @file DistributedArithmetic11.h
 * @brief 分布式算术(ROM分区无乘累加FIR与位串行累加器的硬件友好滤波) — Distributed Arithmetic with MAC-less FIR via ROM Partition and Bit-serial Accumulator for Hardware-friendly Filtering
 *
 * 功能: 实现分布式算术(distributed arithmetic)，采用ROM分区无乘累加FIR(MAC-less FIR via ROM partition)
 *       与位串行累加器(bit-serial accumulator)实现硬件友好滤波(hardware-friendly filtering)。
 *
 * 协作: FIR8(FIR滤波器) / FFT10(FFT) / IIR9(IIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术(ROM分区无乘累加FIR与位串行累加器)
 */
class DistributedArithmetic11 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter configuration */
    struct DAConfig {
        QVector<double> coefficients;
        int inputBits = 16;     // Input word length
        int coeffBits = 16;     // Coefficient quantization bits
        int lutSegments = 4;    // ROM partition count
    };

    /** @brief Filter result */
    struct DAResult {
        QVector<double> output;
        double peakAmplitude = 0.0;
        double processingGain = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DistributedArithmetic11(QObject *parent = nullptr);
    ~DistributedArithmetic11() override;

    void setCoefficients(const QVector<double>& coeffs);
    void setInputBits(int bits);
    void setLUTSegments(int segments);

    /** @brief Initialize LUT from coefficients */
    void buildLUT();

    /** @brief Filter input using distributed arithmetic */
    DAResult filter(const QVector<double>& input);

    /** @brief Quantize value to fixed-point */
    qint32 quantize(double value, int bits) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDone(int n, int order, double peak, double timeMs);

private:
    DAConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precomputed LUT tables (one per segment) */
    QVector<QVector<qint32>> m_lutTables;

    /** @brief Shift register for input samples */
    QVector<qint32> m_shiftReg;

    /** @brief Number of taps */
    int m_numTaps = 0;

    /** @brief Taps per LUT segment */
    int m_tapsPerSegment = 0;

    /** @brief Build a single LUT for a segment of coefficients */
    QVector<qint32> buildSegmentLUT(const QVector<double>& segCoeffs, int addrBits) const;

    /** @brief Bit-serial accumulator: accumulate LUT outputs across bits */
    qint32 bitSerialAccumulate(const QVector<qint32>& regSlice) const;

    /** @brief Saturate to output range */
    double saturate(qint64 value) const;
};
