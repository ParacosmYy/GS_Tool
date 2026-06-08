/**
 * @file AdaptiveFilter5.h
 * @brief 自适应滤波器(分块频域分区+重叠保留长脉冲响应抵消) — Adaptive Filter with Partitioned Block Frequency Domain and Overlap-Save for Long Impulse Response Cancellation
 *
 * 功能: 实现自适应滤波器，使用分块频域分区卷积、
 *       重叠保留法处理长脉冲响应、频域LMS自适应更新。
 *
 * 协作: Compressor4(压缩器) / EnvelopeDetector3(包络检测) / WienerFilter2(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 自适应滤波器(分块频域+重叠保留)
 */
class AdaptiveFilter5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalBlocks = 0;
        int filterLength = 0;
        int blockSize = 0;
        int numPartitions = 0;
        double stepSize = 0.0;
        double errorPower = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AdaptiveFilter5(QObject *parent = nullptr);
    ~AdaptiveFilter5() override;

    /** @brief Set filter parameters */
    void setParameters(int filterLength, int blockSize = 256,
                       double stepSize = 0.1, double regularization = 1e-6);

    /** @brief Process one block: returns error signal (desired - filtered) */
    QVector<double> process(const QVector<double>& input,
                             const QVector<double>& desired);

    /** @brief Apply current filter to input (no adaptation) */
    QVector<double> filter(const QVector<double>& input) const;

    /** @brief Get current filter coefficients (time domain) */
    QVector<double> coefficients() const;

    /** @brief Reset filter state */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int block, double errorPower, double timeMs);

private:
    int m_filterLen = 512;
    int m_blockSize = 256;
    int m_fftSize = 0;
    int m_numPartitions = 0;
    double m_stepSize = 0.1;
    double m_reg = 1e-6;

    // Partitioned frequency-domain coefficients: [partition][bin]
    QVector<QVector<double>> m_wReal;
    QVector<QVector<double>> m_wImag;

    // Overlap-save buffers per partition: [partition][sample]
    QVector<QVector<double>> m_overlapBuf;

    // Previous input blocks for partitioned convolution
    QVector<QVector<double>> m_inputHistory;

    // Running error power estimate
    double m_errorPower = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place radix-2 FFT on complex arrays */
    void fft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief In-place inverse FFT */
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Complex multiply: (ar,ai) *= (br,bi) */
    static void cmul(double& ar, double& ai, double br, double bi);
};
