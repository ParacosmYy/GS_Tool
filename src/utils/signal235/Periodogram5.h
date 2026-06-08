/**
 * @file Periodogram5.h
 * @brief 周期图(Bartlett平均周期图+Welch重叠分段法方差降低) — Periodogram with Bartlett Averaged Periodogram and Welch Overlapped Segment Method for Variance Reduction
 *
 * 功能: 实现周期图(periodogram)功率谱估计，采用Bartlett平均周期图(Bartlett averaged
 *       periodogram)和Welch重叠分段法(Welch overlapped segment method)降低方差。
 *
 * 协作: FFTCore5(FFT核心) / WindowFunction3(窗函数) / SpectrumAnalyzer4(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期图(Bartlett平均周期图+Welch重叠分段法方差降低)
 */
class Periodogram5 : public QObject {
    Q_OBJECT

public:
    /** @brief Estimation method */
    enum Method { Bartlett = 0, Welch = 1 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int segmentLength = 0;
        int overlap = 0;
        int numSegments = 0;
        int fftSize = 0;
        int method = 0;
        double freqResolution = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Periodogram5(QObject *parent = nullptr);
    ~Periodogram5() override;

    /** @brief Configure segment length, overlap, FFT size, and method */
    bool configure(int segLen, int overlap, int fftSize, Method method = Welch);

    /** @brief Estimate power spectral density */
    QVector<double> estimate(const QVector<double>& signal, double sampleRate = 1.0);

    /** @brief Get frequency axis bins */
    QVector<double> frequencyAxis(double sampleRate) const;

    /** @brief Get current PSD estimate */
    QVector<double> psd() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimateCompleted(int numSegments, double freqRes, double timeMs);

private:
    int m_segLen = 256;
    int m_overlap = 128;
    int m_fftSize = 256;
    Method m_method = Welch;
    int m_halfFft = 128;

    QVector<double> m_psd;           // current PSD estimate (one-sided)
    QVector<double> m_window;        // window function
    QVector<double> m_twReal;        // precomputed twiddle cos
    QVector<double> m_twImag;        // precomputed twiddle sin

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im) const;

    /** @brief Precompute Hann window and twiddle factors */
    void precompute();

    /** @brief Apply window to segment */
    void applyWindow(QVector<double>& seg) const;
};
