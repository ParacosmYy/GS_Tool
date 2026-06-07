/**
 * @file Compressor3.h
 * @brief 多频段立体声压缩器(中/侧处理+自适应频段交叉) — Multiband Stereo Compressor with Mid/Side Processing and Adaptive Band Crossover
 *
 * 功能: 实现多频段立体声动态范围压缩，支持中/侧(M/S)编码、
 *       自适应交叉频率和独立频段增益控制。
 *
 * 协作: FIRFilter3(FIR滤波) / IIRFilter4(IIR滤波) / Limiter2(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 多频段立体声压缩器(中/侧+自适应交叉)
 */
class Compressor3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalBlocks = 0;
        int numBands = 0;
        double avgInputLevel = 0.0;
        double avgOutputLevel = 0.0;
        double avgGainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Compressor3(QObject *parent = nullptr);
    ~Compressor3() override;

    void setNumBands(int bands);
    void setCrossoverFreqs(const QVector<double>& freqs);
    void setSampleRate(double sr);
    void setThreshold(double db);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    void setKnee(double db);

    /** @brief Process interleaved stereo samples [L,R,L,R,...] */
    QVector<double> process(const QVector<double>& input);

    /** @brief Encode stereo L/R to mid/side */
    QPair<QVector<double>, QVector<double>> encodeMS(
        const QVector<double>& left, const QVector<double>& right) const;

    /** @brief Decode mid/side back to L/R */
    QPair<QVector<double>, QVector<double>> decodeMS(
        const QVector<double>& mid, const QVector<double>& side) const;

    /** @brief Get per-band gain reduction in dB */
    QVector<double> gainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int bands, double gr, double timeMs);

private:
    int m_numBands = 4;
    double m_sampleRate = 44100.0;
    double m_threshold = -20.0;   // dB
    double m_ratio = 4.0;
    double m_attack = 10.0;       // ms
    double m_release = 100.0;     // ms
    double m_knee = 6.0;          // dB

    QVector<double> m_crossoverFreqs;
    QVector<double> m_bandGR;     // current gain reduction per band

    // Envelope state per band
    QVector<double> m_envMid;
    QVector<double> m_envSide;

    // Linkwitz-Riley crossover coefficients (2nd-order per band edge)
    QVector<QVector<double>> m_lpCoeffs;
    QVector<QVector<double>> m_hpCoeffs;
    QVector<QVector<double>> m_lpState;
    QVector<QVector<double>> m_hpState;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design Linkwitz-Riley crossover filters */
    void designCrossovers();

    /** @brief Apply single biquad filter */
    double biquad(double x, const QVector<double>& coeff, QVector<double>& state);

    /** @brief Compute gain from level with soft knee */
    double compressGain(double levelDB) const;
};
