/**
 * @file ZoomFFT2.h
 * @brief 缩放FFT(频移+低通+抽取高分辨率频谱细化) — Zoom FFT via Frequency Shifting, Lowpass Filtering and Decimation for High-Resolution Spectral Zoom
 *
 * 功能: 实现Zoom FFT算法，支持频率搬移将目标频段移至基带、
 *       低通滤波+降采样抽取、高分辨率频谱细化和可变缩放因子。
 *
 * 协作: FftEngine3(FFT引擎) / SlidingDFT5(滑动DFT) / Goertzel4(Goertzel)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 缩放FFT(频移+低通+抽取高分辨率细化)
 */
class ZoomFFT2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int inputSize = 0;
        int outputSize = 0;
        int decimationFactor = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT2(QObject *parent = nullptr);
    ~ZoomFFT2() override;

    void setInputSize(int N);
    void setSampleRate(double sr);
    void setCenterFrequency(double fc);
    void setZoomBandwidth(double bw);
    void setDecimationFactor(int D);

    /** @brief Execute Zoom FFT on input data */
    QVector<double> transform(const QVector<double>& data);

    /** @brief Get frequency axis for zoomed spectrum */
    QVector<double> frequencyAxis() const;

    /** @brief Get current spectral resolution */
    double resolution() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int outSize, double timeMs);

private:
    int m_inputSize = 1024;
    double m_sampleRate = 44100.0;
    double m_centerFreq = 1000.0;
    double m_bandwidth = 500.0;
    int m_decimation = 4;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Frequency shift input by -fc (heterodyne to baseband) */
    QVector<double> frequencyShift(const QVector<double>& data, double fc) const;

    /** @brief Lowpass FIR filter to isolate target band */
    QVector<double> lowpassFilter(const QVector<double>& data,
                                  double cutoff, int order) const;

    /** @brief Decimate by factor D (keep every D-th sample) */
    QVector<double> decimate(const QVector<double>& data, int D) const;

    /** @brief Standard FFT (Cooley-Tukey radix-2) */
    QVector<double> fft(const QVector<double>& real) const;

    /** @brief Design FIR lowpass coefficients via windowed sinc */
    QVector<double> designLowpass(int order, double cutoff) const;
};
