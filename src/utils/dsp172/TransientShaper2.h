/**
 * @file TransientShaper2.h
 * @brief 瞬态塑形器(包络跟随+多频带瞬态检测) — Transient Shaper with Envelope Follower and Multiband Transient Detection
 *
 * 功能: 实现瞬态塑形处理，支持包络跟随器、多频带瞬态检测、
 *       瞬态增强/衰减控制和增益曲线生成。
 *
 * 协作: DynamicsProcessor3(动态处理) / Equalizer5(均衡器) / Compressor4(压缩器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 瞬态塑形处理器
 */
class TransientShaper2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;           ///< 累计处理帧数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
        double lastPeakTransient = 0.0;    ///< 最近峰值瞬态量
    };

    explicit TransientShaper2(QObject *parent = nullptr);
    ~TransientShaper2() override;

    void setAttack(double ms);
    void setSustain(double ms);
    void setTransientGain(double gainDb);
    void setSustainGain(double gainDb);
    void setBandCount(int bands);

    /**
     * @brief 处理单帧音频
     * @param input 输入采样
     * @return 处理后的采样
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 处理完整缓冲区
     * @param buffer 输入/输出缓冲区(逐帧交织)
     * @param channels 通道数
     */
    void processBuffer(QVector<double>& buffer, int channels);

    /** @brief 获取瞬态包络 */
    QVector<double> transientEnvelope() const;

    /** @brief 获取稳态包络 */
    QVector<double> sustainEnvelope() const;

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(double transientLevel);

private:
    /** @brief 包络跟随器状态 */
    struct EnvelopeState {
        double attackCoeff = 0.0;
        double releaseCoeff = 0.0;
        double envelope = 0.0;
    };

    /** @brief 频带状态 */
    struct BandState {
        double lowCut = 0.0;
        double highCut = 0.0;
        double x1 = 0.0, x2 = 0.0;   ///< biquad state
        double y1 = 0.0, y2 = 0.0;
        EnvelopeState env;
    };

    /** @brief 计算时间常数系数 */
    static double timeConstant(double ms, double sampleRate);

    /** @brief 更新包络跟随器 */
    static double followEnvelope(double input, EnvelopeState& state);

    /** @brief 应用二阶带通滤波 */
    double bandpass(double sample, BandState& band, double sr);

    double m_attackMs = 10.0;
    double m_sustainMs = 150.0;
    double m_transientGainDb = 3.0;
    double m_sustainGainDb = 0.0;
    int m_bandCount = 3;
    double m_sampleRate = 44100.0;

    EnvelopeState m_transientEnv;
    EnvelopeState m_sustainEnv;
    QVector<BandState> m_bands;

    QVector<double> m_transientHistory;
    QVector<double> m_sustainHistory;

    Stats m_stats;
    double m_timeSum = 0.0;
};
