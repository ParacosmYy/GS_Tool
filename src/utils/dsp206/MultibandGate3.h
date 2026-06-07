/**
 * @file MultibandGate3.h
 * @brief 多频段门控(Bark尺度频段分裂+逐频段前瞻增益衰减) — Multiband Gate with Bark-Scale Band Splitting and Per-Band Lookahead Gain Reduction
 *
 * 功能: 实现多频段噪声门，支持Bark心理声学尺度频段分裂、
 *       逐频段前瞻增益衰减和自适应阈值控制。
 *
 * 协作: NoiseGate4(噪声门) / Goertzel6(频率检测) / WindowFunction3(窗函数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段门控(Bark尺度频段分裂+逐频段前瞻增益衰减)
 */
class MultibandGate3 : public QObject {
    Q_OBJECT

public:
    /** @brief Per-band gate state */
    struct BandState {
        double threshold = -40.0;       // dB
        double attack = 1.0;            // ms
        double release = 50.0;          // ms
        double reduction = 0.0;         // current gain reduction dB
        double envelope = 0.0;          // current envelope level
        bool gated = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalBlocks = 0;
        int numBands = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandGate3(QObject *parent = nullptr);
    ~MultibandGate3() override;

    void setSampleRate(int rate);
    void setBlockSize(int size);
    void setNumBands(int n);

    /** @brief Process a block of audio through all bands */
    QVector<double> process(const QVector<double>& input);

    /** @brief Split input into Bark-scale frequency bands */
    QVector<QVector<double>> splitBands(const QVector<double>& input) const;

    /** @brief Compute RMS level in dB for a band */
    double bandLevelDb(const QVector<double>& band) const;

    /** @brief Apply lookahead gain reduction to a band */
    QVector<double> applyLookaheadGate(const QVector<double>& band, BandState& state) const;

    /** @brief Merge processed bands back to single signal */
    QVector<double> mergeBands(const QVector<QVector<double>>& bands) const;

    /** @brief Compute Bark frequency from Hz */
    static double hzToBark(double hz);

    /** @brief Compute Hz from Bark */
    static double barkToHz(double bark);

    const Stats& stats() const { return m_stats; }
    QVector<BandState>& bandStates() { return m_bandStates; }
    void resetStatistics();

signals:
    void processingCompleted(int bands, double timeMs);

private:
    int m_sampleRate = 44100;
    int m_blockSize = 1024;
    int m_numBands = 8;

    QVector<BandState> m_bandStates;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design crossover frequencies for Bark bands */
    QVector<double> designCrossoverFreqs() const;

    /** @brief Simple low-pass filter */
    static QVector<double> lowpass(const QVector<double>& in, double cutoff, int sr);

    /** @brief Simple high-pass filter */
    static QVector<double> highpass(const QVector<double>& in, double cutoff, int sr);
};
