/**
 * @file MultibandCompressor8.h
 * @brief 多段压缩器(线性相位交叉+逐带动态范围控制+自动补偿增益) — Multiband Compressor with Linear Phase Crossover and Per-band Dynamic Range Control with Makeup Gain Automation
 *
 * 功能: 实现多段动态范围压缩器，采用线性相位交叉滤波器(linear phase
 *       crossover)将信号分成多个频段，逐带动态范围控制(per-band DRC)
 *       配合自动补偿增益(makeup gain automation)。
 *
 * 协作: IIRFilter10(IIR滤波器) / FFT4(FFT) / FIRFilter6(FIR滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多段压缩器(线性相位交叉+逐带动态范围控制+自动补偿增益)
 */
class MultibandCompressor8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numBands = 0;
        int blockSize = 0;
        int framesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Per-band compressor parameters */
    struct BandParams {
        double threshold = -20.0;    // dB
        double ratio = 4.0;
        double attack = 10.0;        // ms
        double release = 100.0;      // ms
        double kneeWidth = 6.0;      // dB
        double makeupGain = 0.0;     // dB
    };

    explicit MultibandCompressor8(QObject *parent = nullptr);
    ~MultibandCompressor8() override;

    /** @brief Set number of bands (2-8) */
    void setNumBands(int bands);

    /** @brief Set crossover frequencies in Hz */
    void setCrossoverFreqs(const QVector<double>& freqs);

    /** @brief Set parameters for a specific band */
    void setBandParams(int band, const BandParams& params);

    /** @brief Set sample rate */
    void setSampleRate(double rate);

    /** @brief Process a block of samples */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get per-band RMS levels in dB */
    QVector<double> bandLevels() const;

    /** @brief Get per-band gain reduction in dB */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames, double timeMs);

private:
    int m_numBands = 4;
    double m_sampleRate = 44100.0;
    int m_blockSize = 0;

    QVector<double> m_crossoverFreqs;
    QVector<BandParams> m_bandParams;
    QVector<double> m_bandLevels;
    QVector<double> m_gainReduction;

    // Per-band envelope followers
    QVector<double> m_envelope;
    QVector<double> m_gainState;

    // FIR crossover filter coefficients (linear phase)
    QVector<QVector<double>> m_lowpassCoeffs;
    QVector<QVector<double>> m_highpassCoeffs;

    // Circular buffers for FIR filtering
    QVector<QVector<double>> m_delayLines;
    QVector<int> m_delayIdx;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design linear-phase crossover FIR coefficients */
    void designCrossoverFilters();

    /** @brief Apply FIR filter to a band */
    QVector<double> applyFIR(const QVector<double>& input, int bandIdx, bool lowpass);

    /** @brief Compute gain for a band given input level */
    double computeGain(int band, double inputLevel) const;

    /** @brief Automatic makeup gain estimation */
    double autoMakeupGain(int band) const;
};
