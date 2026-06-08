/**
 * @file DistributedArithmetic7.h
 * @brief 分布式算术(对称系数打包+折叠LUT半尺寸FIR滤波) — Distributed Arithmetic with Symmetric Coefficient Packing and Folded LUT for Half-Size FIR Filter
 *
 * 功能: 实现分布式算术(DA)FIR滤波器，利用对称系数打包将查找表(LUT)尺寸减半，
 *       通过折叠(folded)寻址实现高效的固定系数FIR滤波运算。
 *
 * 协作: FIRFilter2(FIR滤波器) / FFTCore5(FFT核心) / WHT6(Walsh-Hadamard)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术(对称折叠LUT FIR滤波器)
 */
class DistributedArithmetic7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        int lutSize = 0;
        int numSamples = 0;
        int numCoefficients = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DistributedArithmetic7(QObject *parent = nullptr);
    ~DistributedArithmetic7() override;

    /** @brief Initialize with FIR coefficients (symmetric recommended) */
    bool initialize(const QVector<double>& coefficients);

    /** @brief Process input samples through DA FIR filter */
    QVector<double> filter(const QVector<double>& input);

    /** @brief Process single sample */
    double processSample(double sample);

    /** @brief Reset internal state (delay line) */
    void reset();

    /** @brief Get current LUT contents for inspection */
    QVector<double> lutContents() const;

    int order() const { return m_filterOrder; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterCompleted(int samples, double timeMs);

private:
    int m_filterOrder = 0;
    int m_halfOrder = 0;
    int m_lutAddressBits = 0;

    // Folded LUT: 2^addressBits entries
    QVector<double> m_lut;

    // Original coefficients
    QVector<double> m_coeffs;

    // Symmetric-folded coefficients (half size)
    QVector<double> m_foldedCoeffs;

    // Circular delay line
    QVector<double> m_delayLine;
    int m_delayPos = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build folded LUT from symmetric coefficients */
    void buildFoldedLUT();

    /** @brief Quantize input to fixed-point for LUT address */
    int quantizeAddress(const QVector<double>& segment) const;
};
