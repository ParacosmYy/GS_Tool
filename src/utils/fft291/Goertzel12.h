/**
 * @file Goertzel12.h
 * @brief Goertzel算法(滑动窗口与双音多频检测实现窄带信道实时DTMF解码) — Goertzel with Sliding Window and Dual-tone Multi-frequency Detection for Real-time DTMF and Tone Decoding in Narrowband Channels
 *
 * 功能: 实现Goertzel算法(Goertzel algorithm)，采用滑动窗口(sliding window)
 *       与双音多频检测(DTMF detection)实现窄带信道实时音调解码(real-time tone decoding)。
 *
 * 协作: SplitRadixFFT11(分裂基FFT) / SlidingDFT11(滑动DFT) / MixedRadixFFT11(混合基FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel算法(滑动窗口与双音多频检测实现窄带信道实时DTMF解码)
 */
class Goertzel12 : public QObject {
    Q_OBJECT

public:
    /** @brief Detected DTMF tone pair */
    struct DTMFResult {
        int lowFreq = 0;                // Low group frequency (Hz)
        int highFreq = 0;               // High group frequency (Hz)
        QChar digit;                    // Decoded digit/character
        double lowMag = 0.0;            // Low group magnitude
        double highMag = 0.0;           // High group magnitude
        bool valid = false;             // Meets validation criteria
    };

    /** @brief Single-frequency detection result */
    struct ToneResult {
        double frequency = 0.0;
        double magnitude = 0.0;
        double phase = 0.0;
        bool detected = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalBlocks = 0;
        int tonesDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Goertzel12(QObject *parent = nullptr);
    ~Goertzel12() override;

    void setSampleRate(double sr);
    void setBlockSize(int N);
    void setDetectionThreshold(double thresh);

    /** @brief Detect a single target frequency in the input block */
    ToneResult detectTone(const QVector<double>& samples, double targetFreq) const;

    /** @brief Detect DTMF digit from a block of samples */
    DTMFResult detectDTMF(const QVector<double>& samples) const;

    /** @brief Sliding window: push one sample, return current detection */
    ToneResult pushSample(double sample, double targetFreq);

    /** @brief Reset sliding window state */
    void resetSlidingWindow(double targetFreq);

    /** @brief Detect multiple tones simultaneously */
    QVector<ToneResult> detectMultiTone(const QVector<double>& samples,
                                         const QVector<double>& targetFreqs) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void dtmfDetected(QChar digit, double timeMs);
    void toneDetected(double freq, double mag, double timeMs);

private:
    double m_sampleRate = 8000.0;
    int m_blockSize = 205;              // Standard DTMF block size
    double m_threshold = 0.5;
    Stats m_stats;
    double m_timeSum = 0.0;

    // DTMF frequency tables
    static constexpr int DTMF_LOW[4]  = {697, 770, 852, 941};
    static constexpr int DTMF_HIGH[4] = {1209, 1336, 1477, 1633};
    static const QChar DTMF_MAP[4][4];

    // Sliding window state
    struct GoertzelState {
        double s0 = 0.0;
        double s1 = 0.0;
        double s2 = 0.0;
        double coeff = 0.0;
        int count = 0;
        int targetN = 0;
        QVector<double> ringBuf;
        int ringPos = 0;
    };

    GoertzelState m_slideState;

    /** @brief Compute Goertzel coefficient for target frequency */
    double goertzelCoeff(double targetFreq, int N) const;

    /** @brief Run Goertzel on a block for a given coefficient */
    double goertzelMagnitude(const QVector<double>& samples,
                              double coeff, int N) const;

    /** @brief Map DTMF frequency pair to digit */
    QChar mapDTMFDigit(int lowIdx, int highIdx) const;
};
