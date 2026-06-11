/**
 * @file ZoomFFT10.h
 * @brief 缩放FFT(级联复频移与抽取链实现多级窄带频谱分析) — Zoom FFT with Cascaded Complex Frequency Shift and Decimation Chain for Multi-stage Narrowband Spectral Analysis
 *
 * 功能: 实现缩放FFT(Zoom FFT)，采用级联复频移(cascaded complex frequency shift)
 *       与抽取链(decimation chain)实现多级窄带频谱分析(multi-stage narrowband spectral analysis)。
 *
 * 协作: DistributedArithmetic12(分布式算术) / SplitRadixFFT11(分裂基FFT) / GoertzelFilter(戈泽尔滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

class ZoomFFT10 : public QObject {
    Q_OBJECT

public:
    /** @brief Zoom FFT configuration */
    struct ZoomConfig {
        double centerFreqHz = 1000.0;
        double bandwidthHz = 200.0;
        double sampleRateHz = 44100.0;
        int fftSize = 1024;
        int decimationStages = 2;
        int decimationFactor = 4;  // per stage
    };

    /** @brief Spectral result */
    struct SpectrumResult {
        QVector<double> magnitude;
        QVector<double> phase;
        QVector<double> frequencies;
        double binResolutionHz = 0.0;
        int numBins = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int effectiveFFTSize = 0;
        double freqResolution = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ZoomFFT10(QObject *parent = nullptr);
    ~ZoomFFT10() override;

    void setConfig(const ZoomConfig& config);

    /** @brief Analyze narrowband spectrum around center frequency */
    SpectrumResult analyze(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analyzeDone(int bins, double resolution, double timeMs);

private:
    ZoomConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Complex frequency shift: move centerFreq to DC */
    QVector<double> frequencyShift(const QVector<double>& input, double freq, double fs) const;

    /** @brief Cascaded decimation: lowpass + downsample chain */
    QVector<double> decimate(const QVector<double>& input, int factor, int stages) const;

    /** @brief Lowpass FIR filter (windowed sinc) */
    QVector<double> lowpassFilter(const QVector<double>& input, double cutoff, double fs) const;

    /** @brief Downsample by integer factor */
    QVector<double> downsample(const QVector<double>& input, int factor) const;

    /** @brief In-place radix-2 FFT on interleaved complex data [re0,im0,re1,im1,...] */
    void fft(QVector<double>& reIm) const;

    /** @brief Generate frequency axis for zoomed spectrum */
    QVector<double> frequencyAxis(int nBins) const;
};
