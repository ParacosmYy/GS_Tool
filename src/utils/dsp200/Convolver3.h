/**
 * @file Convolver3.h
 * @brief 分区卷积(均匀分区FFT+零延迟频域处理) — Partitioned Convolution with Uniform Partition FFT and Zero-Latency Frequency-Domain Processing
 *
 * 功能: 实现分区卷积算法，支持均匀FFT分区、
 *       零延迟频域处理和长脉冲响应实时卷积。
 *
 * 协作: Convolver2(重叠相加) / FFTW5(FFT引擎) / FilterDesign2(滤波器设计)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分区卷积(均匀分区FFT+零延迟频域处理)
 */
class Convolver3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalBlocks = 0;
        int blockSize = 0;
        int numPartitions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Convolver3(QObject *parent = nullptr);
    ~Convolver3() override;

    void setBlockSize(int size);
    void setImpulseResponse(const QVector<double>& ir);

    /** @brief Process one block of input samples */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief Full convolution of input with stored IR */
    QVector<double> convolve(const QVector<double>& input) const;

    /** @brief Perform FFT on real sequence (in-place complex output) */
    static void fft(QVector<double>& re, QVector<double>& im);

    /** @brief Perform inverse FFT */
    static void ifft(QVector<double>& re, QVector<double>& im);

    /** @brief Complex multiply: (ar,ai) *= (br,bi) */
    static void complexMultiply(double& ar, double& ai, double br, double bi);

    /** @brief Bit-reverse permutation for FFT */
    static void bitReverse(QVector<double>& re, QVector<double>& im);

    /** @brief Precompute IR partitions in frequency domain */
    void precomputePartitions();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int blockIdx, double timeMs);

private:
    int m_blockSize = 64;
    int m_fftSize = 128;
    int m_numPartitions = 0;

    QVector<double> m_ir;
    QVector<QVector<double>> m_irFreqRe;  // IR partition FFT (real)
    QVector<QVector<double>> m_irFreqIm;  // IR partition FFT (imag)

    // Overlap-save circular buffer in frequency domain
    QVector<QVector<double>> m_freqBufRe;
    QVector<QVector<double>> m_freqBufIm;
    int m_bufHead = 0;

    // Time-domain overlap buffer
    QVector<double> m_overlapBuf;

    Stats m_stats;
    double m_timeSum = 0.0;
};
