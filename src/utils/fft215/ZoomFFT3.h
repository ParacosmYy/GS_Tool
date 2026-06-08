/**
 * @file ZoomFFT3.h
 * @brief 缩放FFT(复数频移抽取+多相信道化高分辨频谱缩放) — Zoom FFT with Complex Band-Shift Decimation and Polyphase Channelizer for High-Resolution Spectral Zoom
 *
 * 功能: 实现缩放FFT，支持复数频移、抽取降采样、
 *       多相信道滤波器组和高分辨率频谱缩放分析。
 *
 * 协作: DistributedArithmetic6(分布式算术) / WHT5(沃尔什变换) / FFTAnalyzer(FFT分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT(复数频移+多相信道化)
 */
class ZoomFFT3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int inputSize = 0;
        int fftSize = 0;
        int decimationFactor = 0;
        double centerFreq = 0.0;
        double bandwidth = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT3(QObject *parent = nullptr);
    ~ZoomFFT3() override;

    /** @brief Set zoom parameters */
    void setParameters(int fftSize, double centerFreq, double bandwidth,
                       int decimationFactor, double sampleRate = 44100.0);

    /** @brief Process input and return zoomed spectrum */
    QVector<double> process(const QVector<double>& input);

    /** @brief Process complex input (I/Q data interleaved) */
    QVector<double> processComplex(const QVector<double>& real,
                                    const QVector<double>& imag);

    /** @brief Get frequency axis for zoomed spectrum */
    QVector<double> frequencyAxis() const;

    /** @brief Apply complex frequency shift (heterodyne) */
    void frequencyShift(QVector<double>& real, QVector<double>& imag,
                        double shiftFreq) const;

    /** @brief Apply polyphase channelizer filter and decimate */
    void decimate(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Perform FFT on complex data */
    QVector<double> fftComplex(const QVector<double>& real,
                                const QVector<double>& imag) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double centerFreq, double timeMs);

private:
    int m_fftSize = 1024;
    int m_decimation = 4;
    double m_centerFreq = 0.0;
    double m_bandwidth = 1000.0;
    double m_sampleRate = 44100.0;
    int m_filterOrder = 64;

    // Polyphase filter coefficients
    QVector<double> m_filterCoeffs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design polyphase lowpass filter for decimation */
    void designFilter();

    /** @brief In-place bit-reversal permutation */
    void bitReverse(QVector<double>& real, QVector<double>& imag) const;

    /** @brief Compute twiddle factor */
    static void twiddle(int k, int N, double& wr, double& wi);
};
