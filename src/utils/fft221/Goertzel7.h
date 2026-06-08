/**
 * @file Goertzel7.h
 * @brief Goertzel算法(滑动窗口多音检测+能量比验证) — Goertzel Algorithm with Sliding-Window Multi-Tone Detection and Energy Ratio Validation for DTMF/Multi-Frequency
 *
 * 功能: 实现Goertzel单频点DFT检测，支持滑动窗口多音并行检测，
 *       通过能量比验证区分DTMF及多频信号。
 *
 * 协作: SplitRadixFFT6(FFT) / SlidingDFT6(滑动DFT) / MultibandGate4(多频段门控)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel算法(滑动窗口多音检测+能量比验证)
 */
class Goertzel7 : public QObject {
    Q_OBJECT

public:
    /** @brief Detection result for a single frequency */
    struct ToneResult {
        double frequency = 0.0;
        double magnitude = 0.0;
        double energyRatio = 0.0;
        bool detected = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int sampleRate = 0;
        int numTones = 0;
        int numDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Goertzel7(QObject *parent = nullptr);
    ~Goertzel7() override;

    /** @brief Set parameters: block size, sample rate, target frequencies */
    void setParameters(int blockSize = 205, int sampleRate = 8000,
                       const QVector<double>& targetFreqs = QVector<double>());

    /** @brief Process a block and detect tones */
    QVector<ToneResult> detect(const QVector<double>& samples);

    /** @brief Sliding window: process one new sample, return detections if window full */
    QVector<ToneResult> pushSample(double sample);

    /** @brief Compute single-frequency Goertzel magnitude */
    double goertzelMag(const QVector<double>& samples, double targetFreq) const;

    /** @brief Validate DTMF pair via energy ratio */
    bool validateDTMF(const QVector<ToneResult>& results,
                      double lowFreq, double highFreq,
                      double minRatio = 0.5) const;

    /** @brief Get detected tone frequencies */
    QVector<double> detectedFrequencies() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tonesDetected(int count, double timeMs);

private:
    int m_blockSize = 205;
    int m_sampleRate = 8000;
    QVector<double> m_targetFreqs;

    // Sliding window state
    QVector<double> m_windowBuf;
    int m_windowPos = 0;
    bool m_windowFull = false;

    // Per-frequency Goertzel state for sliding mode
    struct GoertzelState {
        double coeff = 0.0;
        double s0 = 0.0;
        double s1 = 0.0;
        double s2 = 0.0;
        double frequency = 0.0;
    };
    QVector<GoertzelState> m_gStates;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Last detection results
    QVector<ToneResult> m_lastResults;

    /** @brief Compute Goertzel coefficient for a frequency */
    double computeCoeff(double targetFreq) const;

    /** @brief Run Goertzel on a complete block for one frequency */
    double runGoertzelBlock(const QVector<double>& samples, double coeff) const;

    /** @brief Compute total block energy */
    double blockEnergy(const QVector<double>& samples) const;

    /** @brief Reset sliding window Goertzel states */
    void resetSlidingStates();
};
