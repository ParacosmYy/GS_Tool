/**
 * @file MultibandCompressor2.h
 * @brief 多频带压缩器(4频带交叉+独立压缩+补偿增益) — Multiband Compressor with 4-Band Crossover, Independent Ratio/Threshold and Makeup Gain
 *
 * 功能: 实现4频带动态压缩器，支持Linkwitz-Riley交叉滤波器、
 *       各频带独立阈值/比率/启动/释放时间和补偿增益控制。
 *
 * 协作: BiquadFilter5(双二阶滤波器) / PeakMeter3(峰值表) / SignalGenerator2(信号发生)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多频带压缩器
 */
class MultibandCompressor2 : public QObject {
    Q_OBJECT

public:
    /** @brief 单频带参数 */
    struct BandParams {
        double threshold = -20.0;   ///< 阈值(dB)
        double ratio = 4.0;         ///< 压缩比
        double attack = 10.0;       ///< 启动时间(ms)
        double release = 100.0;     ///< 释放时间(ms)
        double makeupGain = 0.0;    ///< 补偿增益(dB)
        double kneeWidth = 6.0;     ///< 软拐点宽度(dB)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;          ///< 累计处理帧数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int sampleRate = 44100;           ///< 采样率
    };

    explicit MultibandCompressor2(QObject *parent = nullptr);
    ~MultibandCompressor2() override;

    void setSampleRate(int rate);
    void setCrossoverFrequencies(double lowMid, double midHigh1, double high1High2);
    void setBandParams(int band, const BandParams& params);

    /**
     * @brief 处理音频帧
     * @param input 输入采样
     * @return 压缩后采样
     */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取各频带增益缩减(dB) */
    QVector<double> bandGainReduction() const;

    /** @brief 获取各频带RMS电平(dB) */
    QVector<double> bandLevels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int frames);
    void gainReductionChanged(double b0, double b1, double b2, double b3);

private:
    /** @brief 二阶IIR交叉滤波器(Linkwitz-Riley 4th-order) */
    struct CrossoverFilter {
        double x1 = 0, x2 = 0, x3 = 0, x4 = 0;
        double y1 = 0, y2 = 0, y3 = 0, y4 = 0;
        double a0 = 1, a1 = 0, a2 = 0, b0 = 1, b1 = 0, b2 = 0;
    };

    /** @brief 增益跟踪器 */
    struct GainEnvelope {
        double gainLin = 1.0;
        double attackCoeff = 0.0;
        double releaseCoeff = 0.0;
        double reductionDb = 0.0;
        double rmsLevel = -120.0;
        double rmsAccum = 0.0;
        int rmsCount = 0;
    };

    /** @brief 设计交叉滤波器系数 */
    void designCrossover(CrossoverFilter& lp, CrossoverFilter& hp, double freq);

    /** @brief 滤波器处理单采样 */
    static double filterProcess(CrossoverFilter& f, double x);

    /** @brief 压缩器增益计算(软拐点) */
    double computeGain(double inputDb, const BandParams& p, GainEnvelope& env);

    int m_sampleRate = 44100;
    double m_xoFreqs[3] = {120.0, 1000.0, 5000.0};

    BandParams m_bands[4];
    CrossoverFilter m_lp[3], m_hp[3];
    GainEnvelope m_env[4];

    Stats m_stats;
    double m_timeSum = 0.0;
};
