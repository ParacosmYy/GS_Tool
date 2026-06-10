/**
 * @file Goertzel10.h
 * @brief Goertzel算法(滑动窗口重叠相加连续实时单频DFT bin提取) — Goertzel Algorithm with Sliding Window and Overlap-add for Continuous Real-time Single-frequency DFT Bin Extraction
 *
 * 功能: 实现Goertzel算法(Goertzel algorithm)，采用滑动窗口(sliding window)和
 *       重叠相加(overlap-add)进行连续实时单频DFT bin提取(continuous real-time
 *       single-frequency DFT bin extraction)。
 *
 * 协作: SplitRadixFFT9(分裂基数FFT) / SlidingDFT9(滑动DFT) / WindowFunction6(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel算法(滑动窗口重叠相加连续实时单频DFT bin提取)
 */
class Goertzel10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowSize = 0;
        int overlapSize = 0;
        int numBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single frequency bin result */
    struct BinResult {
        double frequency = 0.0;
        double magnitude = 0.0;
        double phase = 0.0;
        double powerDb = 0.0;
    };

    explicit Goertzel10(QObject *parent = nullptr);
    ~Goertzel10() override;

    /** @brief Set target sample rate */
    void setSampleRate(double sampleRate);

    /** @brief Set window size and overlap ratio (0.0-0.75) */
    void setWindowConfig(int windowSize, double overlapRatio);

    /** @brief Add a target frequency bin to extract */
    void addTargetFrequency(double frequency);

    /** @brief Set multiple target frequencies at once */
    void setTargetFrequencies(const QVector<double>& frequencies);

    /** @brief Process a block of samples, returns results for each target bin */
    QVector<BinResult> process(const QVector<double>& samples);

    /** @brief Get current windowed buffer size */
    int windowSize() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void binsUpdated(int numBins, int windowSize, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_windowSize = 1024;
    int m_overlapSize = 512;
    QVector<double> m_targetFreqs;

    // Goertzel state per target frequency
    struct GoertzelState {
        double coeff = 0.0;
        double s0 = 0.0, s1 = 0.0, s2 = 0.0;
        double frequency = 0.0;
    };

    QVector<GoertzelState> m_states;

    // Circular buffer for sliding window
    QVector<double> m_buffer;
    int m_bufferPos = 0;
    int m_bufferFill = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Goertzel coefficient for frequency bin */
    double computeCoeff(double frequency) const;

    /** @brief Reset Goertzel IIR state */
    void resetState(GoertzelState& state);

    /** @brief Process one sample through all Goertzel filters */
    void processSample(double sample);

    /** @brief Extract result from Goertzel state */
    BinResult extractResult(const GoertzelState& state) const;

    /** @brief Apply Hann window to buffer */
    void applyWindow(QVector<double>& data) const;
};
