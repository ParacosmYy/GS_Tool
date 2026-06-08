/**
 * @file ZoomFFT4.h
 * @brief 缩放FFT(滑动窗口变换+多相分析滤波器组窄带频谱监测) — Zoom FFT with Sliding Window Transform and Polyphase Analysis Filterbank for Narrowband Spectral Monitoring
 *
 * 功能: 实现缩放FFT(Zoom FFT)，通过滑动窗口变换与多相分析滤波器组
 *       (polyphase analysis filterbank)实现窄带频谱的高分辨率监测。
 *
 * 协作: FFTCore5(FFT核心) / DistributedArithmetic7(分布式算术) / ChirpZ3(Chirp-Z)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT(滑动窗口+多相分析滤波器组)
 */
class ZoomFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        int zoomFactor = 0;
        int numTransforms = 0;
        double centerFreq = 0.0;
        double bandwidth = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT4(QObject *parent = nullptr);
    ~ZoomFFT4() override;

    /** @brief Configure zoom parameters */
    bool configure(double sampleRate, double centerFreq,
                   double bandwidth, int fftSize);

    /** @brief Process input block through zoom FFT */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get frequency bins for current configuration */
    QVector<double> frequencyBins() const;

    /** @brief Reset filter state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumReady(int bins, double centerFreq, double timeMs);

private:
    double m_sampleRate = 44100.0;
    double m_centerFreq = 0.0;
    double m_bandwidth = 0.0;
    int m_fftSize = 256;
    int m_zoomFactor = 1;
    int m_decimationFactor = 1;

    // Polyphase filter coefficients
    QVector<double> m_filterCoeffs;
    int m_filterLength = 0;
    int m_numPhases = 0;

    // Filter state per phase
    QVector<QVector<double>> m_phaseState;
    QVector<int> m_phasePos;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design polyphase analysis filter */
    void designPolyphaseFilter();

    /** @brief Frequency shift (heterodyne) input to baseband */
    QVector<double> heterodyne(const QVector<double>& input) const;

    /** @brief Decimate with polyphase filtering */
    QVector<double> polyphaseDecimate(const QVector<double>& input);

    /** @brief In-place DFT (small size) */
    QVector<double> computeDFT(const QVector<double>& input) const;
};
