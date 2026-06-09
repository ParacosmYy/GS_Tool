/**
 * @file Goertzel9.h
 * @brief Goertzel算法(多音并行检测+可配置跳距滑动窗口) — Goertzel Algorithm with Multi-Tone Parallel Detection and Sliding Window with Configurable Hop Size for Streaming DTMF
 *
 * 功能: 实现Goertzel算法(Goertzel Algorithm)，支持多音并行检测
 *       (multi-tone parallel detection)，使用可配置跳距的滑动窗口
 *       (sliding window with configurable hop size)适配流式DTMF解码。
 *
 * 协作: SplitRadixFFT8(分裂基FFT) / MixedRadixFFT8(混合基FFT) / SlidingDFT8(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel算法(多音并行检测+滑动窗口DTMF)
 */
class Goertzel9 : public QObject {
    Q_OBJECT

public:
    /** @brief Tone detection result */
    struct ToneResult {
        double frequency = 0.0;
        double magnitude = 0.0;
        double magnitudeDb = -120.0;
        double phase = 0.0;
        bool detected = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int numTones = 0;
        int numDetections = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Goertzel9(QObject *parent = nullptr);
    ~Goertzel9() override;

    /** @brief Set target frequencies for parallel detection */
    void setTargetFreqs(const QVector<double>& freqs, double sampleRate, int blockSize);

    /** @brief Set detection threshold in dB */
    void setThresholdDb(double threshold);

    /** @brief Set sliding window hop size */
    void setHopSize(int hop);

    /** @brief Process a block and detect all target tones */
    QVector<ToneResult> processBlock(const QVector<double>& samples);

    /** @brief Process streaming samples (sliding window) */
    QVector<ToneResult> processStreaming(const QVector<double>& samples);

    /** @brief Get DTMF row/col frequencies result */
    QVector<ToneResult> detectDTMF(const QVector<double>& samples);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tonesDetected(int numDetected, double maxMagDb, double timeMs);

private:
    double m_sampleRate = 8000.0;
    int m_blockSize = 205;
    int m_hopSize = 102;
    double m_thresholdDb = -30.0;

    /** @brief Per-tone Goertzel coefficients */
    struct ToneCoeffs {
        double freq;
        double coeff;
        double coeff2;
        double sineVal;
        double cosineVal;
    };

    QVector<ToneCoeffs> m_tones;

    /** @brief Sliding window ring buffer */
    QVector<double> m_ringBuf;
    int m_ringPos = 0;
    int m_ringFill = 0;

    /** @brief Running Goertzel state per tone */
    struct GoertzelState {
        double s0 = 0.0;
        double s1 = 0.0;
        double s2 = 0.0;
    };

    QVector<GoertzelState> m_states;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute Goertzel coefficients for a frequency */
    ToneCoeffs computeCoeffs(double freq, double sampleRate, int N) const;

    /** @brief Run single Goertzel on a block for one tone */
    ToneResult goertzelSingle(const QVector<double>& block,
                               const ToneCoeffs& tc) const;

    /** @brief Compute magnitude and phase from Goertzel final state */
    ToneResult extractResult(double s1, double s2,
                              const ToneCoeffs& tc) const;

    /** @brief Initialize ring buffer */
    void initRing();
};
