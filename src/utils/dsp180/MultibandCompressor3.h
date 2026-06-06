/**
 * @file MultibandCompressor3.h
 * @brief 多频段压缩器(Linkwitz-Riley 4阶分频+每带动态+Mid/Side) — Multiband Compressor with Linkwitz-Riley 4th-order Crossover, Per-band Dynamics and Mid/Side Processing
 *
 * 功能: 实现多频段动态压缩器，支持Linkwitz-Riley 4阶交叉分频、
 *       每频段独立压缩/扩展、Mid/Side处理和增益补偿。
 *
 * 协作: IIRFilter3(IIR滤波) / WindowedFIR3(FIR窗) / PeakDetector3(峰值检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频段压缩器(LR4分频+每带动态+Mid/Side)
 */
class MultibandCompressor3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProcessed = 0;
        int numBands = 0;
        int blockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 每频段压缩参数 */
    struct BandParams {
        double threshold = -20.0;   ///< Threshold in dB
        double ratio = 4.0;         ///< Compression ratio
        double attack = 10.0;       ///< Attack time in ms
        double release = 100.0;     ///< Release time in ms
        double knee = 6.0;          ///< Knee width in dB
        double makeupGain = 0.0;    ///< Makeup gain in dB
        bool enabled = true;
    };

    explicit MultibandCompressor3(QObject *parent = nullptr);
    ~MultibandCompressor3() override;

    void setSampleRate(double sr);
    void setNumBands(int bands);
    void setCrossoverFreqs(const QVector<double>& freqs);
    void setBandParams(int band, const BandParams& params);
    void setMidSideEnabled(bool enabled);

    /** @brief 处理单声道/立体声数据 */
    QVector<QVector<double>> process(const QVector<QVector<double>>& input);

    /** @brief 获取各频段RMS电平(dB) */
    QVector<double> bandLevels() const;

    /** @brief 获取各频段增益削减(dB) */
    QVector<double> bandGainReduction() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int blockSize, double peakReduction);

private:
    double m_sampleRate = 44100.0;
    int m_numBands = 4;
    bool m_midSide = false;

    QVector<double> m_crossoverFreqs;
    QVector<BandParams> m_bandParams;

    // Filter state: [band][biquad-section][state]
    struct BiquadState {
        double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    };
    struct LR4State {
        BiquadState lp[2]; // Two cascaded biquads for LP
        BiquadState hp[2]; // Two cascaded biquads for HP
    };
    QVector<LR4State> m_filterState;

    // Envelope followers per band
    QVector<double> m_envLevel;
    // Per-band gain reduction
    QVector<double> m_gainReduction;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Linkwitz-Riley 4th-order coefficients */
    void computeLR4Coeffs(double freq, double b0[5], double a0[5],
                          double b1[5], double a1[5]) const;

    /** @brief Apply biquad filter section */
    double biquadProcess(double in, BiquadState& s,
                         const double b[3], const double a[3]);

    /** @brief Apply LR4 crossover for one band */
    void crossoverFilter(const QVector<double>& in,
                          QVector<double>& lpOut, QVector<double>& hpOut,
                          int bandIdx);

    /** @brief Compute gain reduction for given level */
    double computeGain(double levelDb, const BandParams& p) const;

    /** @brief Mid/Side encoding */
    void encodeMS(const QVector<double>& L, const QVector<double>& R,
                  QVector<double>& mid, QVector<double>& side);
    void decodeMS(const QVector<double>& mid, const QVector<double>& side,
                  QVector<double>& L, QVector<double>& R);
};
