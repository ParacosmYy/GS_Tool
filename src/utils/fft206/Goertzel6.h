/**
 * @file Goertzel6.h
 * @brief Goertzel算法(并行多音检测引擎+能量DTMF验证) — Goertzel Algorithm with Parallel Multi-Tone Detection Engine and Energy-Based DTMF Validation
 *
 * 功能: 实现Goertzel频率检测算法，支持并行多音检测、
 *       基于能量的DTMF验证和实时按键音识别。
 *
 * 协作: SplitRadixFFT5(FFT) / NoiseGate4(噪声门) / Periodogram3(周期图)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Goertzel算法(并行多音检测引擎+能量DTMF验证)
 */
class Goertzel6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalDetections = 0;
        int blockSize = 0;
        int sampleRate = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief DTMF tone pair result */
    struct DtmfResult {
        QString digit;
        double lowFreqEnergy = 0.0;
        double highFreqEnergy = 0.0;
        bool valid = false;
    };

    explicit Goertzel6(QObject *parent = nullptr);
    ~Goertzel6() override;

    void setSampleRate(int rate);
    void setBlockSize(int N);

    /** @brief Compute Goertzel magnitude for a single frequency */
    double goertzelMag(const QVector<double>& samples, double targetFreq) const;

    /** @brief Detect multiple frequencies in parallel */
    QVector<QPair<double, double>> detectMultiTone(const QVector<double>& samples,
                                                    const QVector<double>& frequencies) const;

    /** @brief Detect DTMF digit from audio samples */
    DtmfResult detectDTMF(const QVector<double>& samples) const;

    /** @brief Energy-based DTMF validation */
    bool validateDTMF(double lowEnergy, double highEnergy,
                       double totalEnergy, double twistThreshold) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void toneDetected(double frequency, double magnitude, double timeMs);

private:
    int m_sampleRate = 8000;
    int m_blockSize = 205;

    // DTMF frequencies
    static constexpr double DTMF_LOW[4]  = {697.0, 770.0, 852.0, 941.0};
    static constexpr double DTMF_HIGH[4] = {1209.0, 1336.0, 1477.0, 1633.0};
    static constexpr char DTMF_MAP[4][4] = {
        {'1','2','3','A'}, {'4','5','6','B'},
        {'7','8','9','C'}, {'*','0','#','D'}
    };

    Stats m_stats;
    double m_timeSum = 0.0;
};
