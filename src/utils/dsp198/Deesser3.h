/**
 * @file Deesser3.h
 * @brief 去齿音器(AI引导频谱平坦度+共振峰跟踪齿音检测) — De-esser with AI-Guided Sibilance Detection via Spectral Flatness and Formant Tracking
 *
 * 功能: 实现去齿音处理器，支持频谱平坦度分析、
 *       共振峰跟踪和自适应齿音抑制。
 *
 * 协作: Equalizer5(均衡器) / Compressor4(压缩器) / SpectralGate3(频谱门)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 去齿音器(频谱平坦度+共振峰跟踪)
 */
class Deesser3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int sibilanceFrames = 0;
        double avgReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Detection result for one frame */
    struct DetectionResult {
        bool isSibilant = false;
        double spectralFlatness = 0.0;
        double formantFreq1 = 0.0;
        double formantFreq2 = 0.0;
        double sibilanceEnergy = 0.0;
        double gainReduction = 0.0;
    };

    explicit Deesser3(QObject *parent = nullptr);
    ~Deesser3() override;

    void setThreshold(double db);
    void setFrequencyRange(double lowHz, double highHz);
    void setReductionAmount(double db);
    void setAttackMs(double ms);
    void setReleaseMs(double ms);
    void setSampleRate(double rate);
    void setFftSize(int size);

    /** @brief Process a frame of audio samples */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Detect sibilance in frequency domain data */
    DetectionResult detectSibilance(const QVector<double>& magnitude) const;

    /** @brief Compute spectral flatness for a band */
    double spectralFlatness(const QVector<double>& magnitude, int loBin, int hiBin) const;

    /** @brief Track formants from spectral peaks */
    QPair<double, double> trackFormants(const QVector<double>& magnitude) const;

    /** @brief Get frequency bin index for Hz value */
    int freqToBin(double hz) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sibilanceDetected(double freqHz, double reduction, double timeMs);

private:
    double m_threshold = -20.0;     // dB threshold
    double m_freqLow = 4000.0;      // Hz
    double m_freqHigh = 10000.0;    // Hz
    double m_reduction = 12.0;      // dB
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    double m_sampleRate = 44100.0;
    int m_fftSize = 2048;

    double m_envelope = 0.0;        // current gain envelope
    double m_totalReduction = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute attack/release coefficient */
    double coeffFromMs(double ms) const;

    /** @brief Hann window */
    static QVector<double> hannWindow(int n);

    /** @brief Internal FFT (radix-2) */
    void fft(QVector<double>& re, QVector<double>& im, bool inv) const;
};
