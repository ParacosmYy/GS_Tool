/**
 * @file WHT11.h
 * @brief 沃尔什-哈达玛变换(序列排序快速沃尔什蝶形与二进制移位实现二值频谱分析) — Walsh-Hadamard Transform with Sequency-ordered Fast Walsh Butterflies and Dyadic Shift for Binary Spectral Analysis
 *
 * 功能: 实现沃尔什-哈达玛变换(Walsh-Hadamard transform)，采用序列排序快速沃尔什蝶形(sequency-ordered fast Walsh butterflies)
 *       与二进制移位(dyadic shift)实现二值频谱分析(binary spectral analysis)。
 *
 * 协作: DST12(离散正弦变换) / DCT12(离散余弦变换) / SplitRadixFFT11(分裂基FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

class WHT11 : public QObject {
    Q_OBJECT

public:
    /** @brief Transform result */
    struct WHTResult {
        QVector<double> coefficients;
        double peakValue = 0.0;
        double totalEnergy = 0.0;
        double sequencyEntropy = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WHT11(QObject *parent = nullptr);
    ~WHT11() override;

    /** @brief Set transform size (rounded to power of 2) */
    void setSize(int N);

    /** @brief Forward WHT (natural order) */
    WHTResult forward(const QVector<double>& input);

    /** @brief Forward WHT with sequency (Hadamard) ordering */
    WHTResult forwardSequency(const QVector<double>& input);

    /** @brief Inverse WHT (same as forward, scaled by 1/N) */
    QVector<double> inverse(const QVector<double>& coefficients);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformDone(int N, double peak, double timeMs);

private:
    int m_N = 256;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bit-reversal permutation index for sequency ordering */
    QVector<int> m_sequencyIndex;

    /** @brief Precompute sequency (Hadamard) order index */
    void precomputeSequency();

    /** @brief In-place fast Walsh-Hadamard butterfly (Hadamard order) */
    void fastWHT(QVector<double>& data) const;

    /** @brief Convert natural order to sequency order via Gray code */
    QVector<double> toSequencyOrder(const QVector<double>& data) const;

    /** @brief Compute sequency entropy */
    double computeEntropy(const QVector<double>& coeffs) const;
};
