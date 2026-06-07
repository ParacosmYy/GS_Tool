/**
 * @file Goertzel5.h
 * @brief Goertzel算法(滑动窗口实时音调检测+双音多频DTMF解码) — Goertzel Algorithm with Sliding Window for Real-Time Tone Detection and DTMF Decoding
 *
 * 功能: 实现Goertzel算法，支持滑动窗口实时音调检测、
 *       双音多频(DTMF)解码和可配置目标频率/窗口大小。
 *
 * 协作: ZoomFFT2(缩放FFT) / FftEngine3(FFT引擎) / Periodogram2(周期图)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Goertzel算法(滑动窗口+DTMF解码)
 */
class Goertzel5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDetections = 0;
        int windowSize = 0;
        int sampleRate = 0;
        int dtmfDigits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief DTMF frequency pair result */
    struct DTMFResult {
        QChar digit;
        double rowFreq = 0.0;
        double colFreq = 0.0;
        double rowMag = 0.0;
        double colMag = 0.0;
        double confidence = 0.0;
    };

    explicit Goertzel5(QObject *parent = nullptr);
    ~Goertzel5() override;

    void setSampleRate(int sr);
    void setWindowSize(int N);

    /** @brief Compute magnitude at a single target frequency */
    double detectFrequency(const QVector<double>& data, double targetFreq) const;

    /** @brief Detect multiple frequencies simultaneously */
    QVector<QPair<double, double>> detectFrequencies(
        const QVector<double>& data,
        const QVector<double>& targets) const;

    /** @brief Decode DTMF tone from audio data */
    DTMFResult decodeDTMF(const QVector<double>& data);

    /** @brief Decode a sequence of DTMF digits from audio */
    QString decodeDTMFSequence(const QVector<double>& data, int frameStep = 0);

    /** @brief Sliding window detection: detect tone in streaming data */
    QVector<double> slidingDetect(const QVector<double>& samples,
                                   double targetFreq, int hopSize = 0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void detectionCompleted(double freq, double magnitude, double timeMs);
    void dtmfDecoded(const QChar& digit, double confidence);

private:
    int m_sampleRate = 8000;
    int m_windowSize = 205;

    Stats m_stats;
    double m_timeSum = 0.0;

    // DTMF standard frequencies
    static constexpr double DTMF_ROWS[4] = {697.0, 770.0, 852.0, 941.0};
    static constexpr double DTMF_COLS[4] = {1209.0, 1336.0, 1477.0, 1633.0};
    static constexpr char DTMF_MAP[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    /** @brief Core Goertzel computation for one frequency bin */
    double goertzel(const QVector<double>& data, double targetFreq) const;

    /** @brief Find dominant frequency index in array */
    int findDominant(const QVector<double>& mags) const;
};
