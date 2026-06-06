/**
 * @file Limiter4.h
 * @brief 砖墙限制器(真峰值检测+4x过采样+前瞻+自动释放) — Brick-wall Limiter with True-Peak Detection (4x Oversampling), Lookahead and Auto-Release
 *
 * 功能: 实现砖墙限制器，支持4倍过采样真峰值检测、前瞻缓冲、
 *       自动增益衰减/释放和零延迟增益平滑。
 *
 * 协作: Compressor4(动态压缩) / Equalizer3(均衡器) / DeEsser4(齿音消除)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 砖墙限制器(真峰值检测+前瞻+自动释放)
 */
class Limiter4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;
        double peakReduction = 0.0;
        double truePeak = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Limiter4(QObject *parent = nullptr);
    ~Limiter4() override;

    void setThreshold(double dB);
    void setCeiling(double dB);
    void setLookaheadMs(double ms);
    void setReleaseMs(double ms);
    void setAutoRelease(bool enabled);
    void setSampleRate(double rate);

    /** @brief 处理音频帧，返回限制后的采样 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 重置内部状态 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void limitingOccurred(double reductionDb);

private:
    double m_threshold = -1.0;   ///< Threshold in dB
    double m_ceiling = -0.3;     ///< Ceiling in dB
    double m_lookaheadMs = 5.0;
    double m_releaseMs = 50.0;
    bool m_autoRelease = false;
    double m_sampleRate = 44100.0;

    // Internal state
    double m_gain = 1.0;         ///< Current gain reduction (linear)
    double m_thresholdLin = 0.0; ///< Threshold in linear
    double m_ceilingLin = 0.0;
    int m_lookaheadSamples = 0;
    QVector<double> m_delayLine; ///< Lookahead delay buffer
    int m_delayPos = 0;

    // 4x oversampling FIR coefficients (linear-phase LPF)
    QVector<double> m_osCoeffs;
    int m_osHalfLen = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Design 4x oversampling lowpass filter */
    void designOversamplingFilter();

    /** @brief 4x oversample a single sample, return 4 outputs */
    QVector<double> oversample(double x) const;

    /** @brief Compute true peak via 4x oversampled absolute max */
    double truePeak(const QVector<double>& frame) const;

    /** @brief Compute auto-release based on signal */
    double computeAutoRelease(double peak) const;
};
