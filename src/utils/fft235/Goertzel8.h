/**
 * @file Goertzel8.h
 * @brief Goertzel算法(滑动窗口+可配置重叠实时音调检测) — Goertzel Algorithm with Sliding Window Implementation and Configurable Overlap for Real-Time Tone Detection
 *
 * 功能: 实现Goertzel算法(Goertzel algorithm)，采用滑动窗口(sliding window)实现，
 *       支持可配置重叠(configurable overlap)，用于实时单频音调检测(real-time tone
 *       detection)场景，如DTMF解码和频率监控。
 *
 * 协作: SplitRadixFFT7(分裂基FFT) / MultibandGate4(多频段门控) / WindowFunction4(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel算法(滑动窗口+可配置重叠实时音调检测)
 */
class Goertzel8 : public QObject {
    Q_OBJECT

public:
    /** @brief Detection result for a target frequency */
    struct Detection {
        double frequency = 0.0;
        double magnitude = 0.0;
        double magnitudeDb = 0.0;
        double phase = 0.0;
        bool detected = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int numTargets = 0;
        int sampleRate = 0;
        int overlapsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Goertzel8(QObject *parent = nullptr);
    ~Goertzel8() override;

    /** @brief Configure: block size N, sample rate */
    bool configure(int blockSize, int sampleRate);

    /** @brief Set target frequencies for detection */
    void setTargets(const QVector<double>& frequencies);

    /** @brief Set overlap ratio (0.0 = no overlap, 0.75 = 75% overlap) */
    void setOverlap(double ratio);

    /** @brief Set detection threshold in dB */
    void setThreshold(double thresholdDb);

    /** @brief Process a block of samples and detect tones */
    QVector<Detection> process(const QVector<double>& samples);

    /** @brief Process streaming: feed samples incrementally */
    QVector<Detection> pushSamples(const QVector<double>& samples);

    /** @brief Get target frequencies */
    QVector<double> targets() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(int blockNum, int numDetected);
    void toneDetected(double frequency, double magnitudeDb);

private:
    int m_blockSize = 0;
    int m_sampleRate = 0;
    double m_overlapRatio = 0.0;
    double m_thresholdDb = -30.0;

    QVector<double> m_targets;
    // Precomputed coefficients per target
    QVector<double> m_coeff;       // 2*cos(2*pi*k/N)
    QVector<double> m_cosK;        // cos(2*pi*k/N)
    QVector<double> m_sinK;        // sin(2*pi*k/N)

    // Sliding window buffer
    QVector<double> m_ringBuffer;
    int m_writePos = 0;
    int m_samplesInBuffer = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
    int m_blockNum = 0;

    /** @brief Precompute Goertzel coefficients for each target */
    void precomputeCoefficients();

    /** @brief Run Goertzel on a single frequency */
    Detection goertzelSingle(const QVector<double>& samples, int targetIdx) const;

    /** @brief Check if buffer has enough samples for a block */
    bool hasFullBlock() const;

    /** @brief Extract a contiguous block from ring buffer */
    QVector<double> extractBlock() const;
};
