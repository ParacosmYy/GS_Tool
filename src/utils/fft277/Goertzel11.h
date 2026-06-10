/**
 * @file Goertzel11.h
 * @brief Goertzel算法(广义双音检测与滑动窗能量累积的多频DTMF分析) — Goertzel Algorithm with Generalized Dual-tone Detection and Sliding Window Energy Accumulation for Multi-frequency DTMF Analysis
 *
 * 功能: 实现Goertzel算法(Goertzel algorithm)，采用广义双音检测(generalized dual-tone detection)
 *       与滑动窗能量累积(sliding window energy accumulation)实现多频DTMF分析(multi-frequency DTMF analysis)。
 *
 * 协作: SplitRadixFFT10(分裂基数FFT) / MixedRadixFFT10(混合基数FFT) / WindowFunction7(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel算法(广义双音检测与滑动窗能量累积)
 */
class Goertzel11 : public QObject {
    Q_OBJECT

public:
    /** @brief Detected tone result */
    struct ToneResult {
        double frequency = 0.0;      // Detected frequency
        double magnitude = 0.0;      // Magnitude
        double energy = 0.0;         // Accumulated energy
        double phase = 0.0;          // Phase in radians
        bool detected = false;       // Above threshold
    };

    /** @brief DTMF digit result */
    struct DTMFResult {
        QChar digit;                  // Detected digit
        double rowFreq = 0.0;        // Row group frequency
        double colFreq = 0.0;        // Column group frequency
        double rowMag = 0.0;
        double colMag = 0.0;
        double snr = 0.0;            // Signal-to-noise ratio
        bool valid = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int blockSize = 0;
        int numTones = 0;
        int numDTMFDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Goertzel11(QObject *parent = nullptr);
    ~Goertzel11() override;

    /** @brief Set block size N for Goertzel computation */
    void setBlockSize(int n);

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Set detection threshold in dB */
    void setThreshold(double threshDb);

    /** @brief Detect energy at a specific frequency */
    ToneResult detectTone(const QVector<double>& samples, double targetFreq);

    /** @brief Detect multiple tones simultaneously */
    QVector<ToneResult> detectMultiTone(const QVector<double>& samples,
                                         const QVector<double>& targetFreqs);

    /** @brief Detect DTMF digit from signal */
    DTMFResult detectDTMF(const QVector<double>& samples);

    /** @brief Continuous sliding window DTMF detection over buffer */
    QVector<DTMFResult> slidingDTMF(const QVector<double>& buffer, int hopSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void toneDetected(double freq, double magnitude, double energy, double timeMs);
    void dtmfDetected(const QChar& digit, double rowFreq, double colFreq, double timeMs);

private:
    int m_blockSize = 205;       // Standard DTMF Goertzel block size at 8kHz
    double m_sampleRate = 8000.0;
    double m_thresholdDb = -30.0;

    // DTMF frequencies
    static constexpr double DTMF_ROW[4] = {697.0, 770.0, 852.0, 941.0};
    static constexpr double DTMF_COL[4] = {1209.0, 1336.0, 1477.0, 1633.0};
    static constexpr char DTMF_MAP[4][4] = {
        {'1','2','3','A'}, {'4','5','6','B'},
        {'7','8','9','C'}, {'*','0','#','D'}
    };

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Core Goertzel computation for one frequency bin */
    void goertzelCore(const QVector<double>& samples, double targetFreq,
                      double& real, double& imag, double& energy) const;

    /** @brief Compute sliding window energy for a frequency */
    double slidingEnergy(const QVector<double>& buffer, double targetFreq,
                          int start, int length) const;
};
