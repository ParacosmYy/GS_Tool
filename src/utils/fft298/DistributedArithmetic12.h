/**
 * @file DistributedArithmetic12.h
 * @brief 分布式算术(LUT系数存储与位串行累加实现无乘法器FIR滤波评估) — Distributed Arithmetic with LUT-based Coefficient Storage and Bit-serial Accumulation for Multiplierless FIR Filter Evaluation
 *
 * 功能: 实现分布式算术(distributed arithmetic)，采用LUT系数存储(LUT-based coefficient storage)
 *       与位串行累加(bit-serial accumulation)实现无乘法器FIR滤波评估(multiplierless FIR filter evaluation)。
 *
 * 协作: WHT11(沃尔什-哈达玛变换) / DCT12(离散余弦变换) / SplitRadixFFT11(分裂基FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class DistributedArithmetic12 : public QObject {
    Q_OBJECT

public:
    /** @brief Filter configuration */
    struct FilterConfig {
        int numTaps = 16;
        int inputWidth = 12;      // input bit width
        bool isSigned = true;
    };

    /** @brief Processing result */
    struct FilterResult {
        QVector<double> output;
        double peakValue = 0.0;
        double rmsValue = 0.0;
        int numSamples = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFilters = 0;
        int numTaps = 0;
        int lutSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DistributedArithmetic12(QObject *parent = nullptr);
    ~DistributedArithmetic12() override;

    /** @brief Set filter coefficients (triggers LUT rebuild) */
    void setCoefficients(const QVector<double>& coeffs);

    /** @brief Set filter configuration */
    void setConfig(const FilterConfig& config);

    /** @brief Process input samples through DA FIR filter */
    FilterResult filter(const QVector<double>& input);

    /** @brief Get the precomputed LUT */
    const QVector<double>& lut() const { return m_lut; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterDone(int n, double peak, double timeMs);

private:
    FilterConfig m_config;
    QVector<double> m_coefficients;
    QVector<double> m_lut;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Number of LUT address bits = log2(numTaps) rounded up */
    int m_lutAddrBits = 0;

    /** @brief Build LUT: for each possible address word, compute partial sum */
    void buildLUT();

    /** @brief Quantize input to fixed-point integer representation */
    QVector<qint64> quantizeInput(const QVector<double>& input) const;

    /** @brief Bit-serial DA accumulation */
    QVector<double> daAccumulate(const QVector<qint64>& quantized) const;
};
