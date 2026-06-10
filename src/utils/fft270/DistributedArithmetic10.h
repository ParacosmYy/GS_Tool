/**
 * @file DistributedArithmetic10.h
 * @brief 分布式算术(查找表分解与位串行累加无乘法器FIR滤波) — Distributed Arithmetic with Lookup Table Decomposition and Bit-Serial Accumulation for Multiplierless FIR Filtering
 *
 * 功能: 实现分布式算术(Distributed arithmetic)，采用查找表分解(LUT decomposition)
 *       与位串行累加(bit-serial accumulation)实现无乘法器FIR滤波(multiplierless FIR)。
 *
 * 协作: WHT9(沃尔什变换) / FFT10(快速傅里叶变换) / FIRFilter7(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术(查找表分解与位串行累加无乘法器FIR滤波)
 */
class DistributedArithmetic10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        int lutSize = 0;
        int inputWordLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Coefficient quantization mode */
    enum QuantMode {
        Truncate = 0,
        Round,
        Convergent
    };

    explicit DistributedArithmetic10(QObject *parent = nullptr);
    ~DistributedArithmetic10() override;

    /** @brief Set FIR filter coefficients (floating-point) */
    void setCoefficients(const QVector<double>& coeffs);

    /** @brief Set input word length in bits (for LUT address width) */
    void setInputWordLength(int bits);

    /** @brief Set quantization mode for coefficient conversion */
    void setQuantMode(QuantMode mode);

    /** @brief Process input samples through DA FIR filter */
    QVector<double> filter(const QVector<double>& input);

    /** @brief Get the precomputed LUT contents */
    QVector<double> lookupTable() const;

    /** @brief Get quantized coefficients */
    QVector<qint32> quantizedCoeffs() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterCompleted(int numSamples, int order, double timeMs);

private:
    int m_wordLength = 16;
    QuantMode m_quantMode = Round;

    QVector<double> m_floatCoeffs;
    QVector<qint32> m_quantCoeffs;
    QVector<double> m_lut;          // 2^N entries for N-tap LUT

    int m_order = 0;
    int m_lutAddrBits = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build LUT from quantized coefficients */
    void buildLUT();

    /** @brief Quantize a floating-point coefficient to fixed-point */
    qint32 quantize(double value) const;
};
