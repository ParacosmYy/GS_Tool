/**
 * @file Limiter3.h
 * @brief 砖墙限幅器(前视缓冲+真峰值检测+增益平滑) — Brick-Wall Limiter with Lookahead Buffer, True Peak Detection and Gain Smoothing
 *
 * 功能: 实现砖墙限幅器，支持前视延迟缓冲、真峰值(True Peak)过零点检测、
 *       增益平滑(attack/release)控制。
 *
 * 协作: Compressor3(压缩器) / Equalizer4(均衡器) / SignalGenerator(信号发生器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 砖墙限幅器
 */
class Limiter3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;          ///< 累计处理采样数
        double peakReductionDb = 0.0;      ///< 最近峰值衰减量(dB)
        double avgGainReduction = 0.0;     ///< 平均增益衰减量
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit Limiter3(QObject *parent = nullptr);
    ~Limiter3() override;

    void setThreshold(double db);
    void setCeiling(double db);
    void setAttack(double ms);
    void setRelease(double ms);
    void setLookahead(int samples);
    void setSampleRate(double rate);

    /**
     * @brief 处理单声道音频
     * @param input 输入采样
     * @return 限幅后的采样
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 处理立体声音频(交错L/R)
     * @param input 交错输入 [L0,R0,L1,R1,...]
     * @return 限幅后的采样
     */
    QVector<double> processStereo(const QVector<double>& input);

    /** @brief 真峰值检测: 4x过采样插值 */
    double truePeak(const QVector<double>& buffer, int idx, int size) const;

    /** @brief 重置内部状态 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double peakReductionDb);

private:
    /** @brief dB <-> 线性增益转换 */
    static double dbToLinear(double db);
    static double linearToDb(double lin);

    /** @brief 计算增益衰减量 */
    double computeGainReduction(double peak) const;

    /** @brief 平滑增益变化(envelope follower) */
    void smoothGain(double& currentGain, double targetGain, int samples);

    double m_thresholdDb = -1.0;   ///< 阈值(dB)
    double m_ceilingDb = -0.3;     ///< 输出上限(dB)
    double m_attackMs = 1.0;       ///< 启动时间(ms)
    double m_releaseMs = 50.0;     ///< 释放时间(ms)
    int m_lookahead = 64;          ///< 前视缓冲采样数
    double m_sampleRate = 44100.0;

    double m_thresholdLin = 0.0;   ///< 阈值(线性)
    double m_ceilingLin = 0.0;     ///< 上限(线性)
    double m_gainSmooth = 1.0;     ///< 当前平滑增益
    double m_attackCoeff = 0.0;    ///< 启动系数
    double m_releaseCoeff = 0.0;   ///< 释放系数

    QVector<double> m_delayBufferL; ///< 左声道前视延迟
    QVector<double> m_delayBufferR; ///< 右声道前视延迟
    int m_delayPos = 0;             ///< 延迟缓冲写位置

    Stats m_stats;
    double m_timeSum = 0.0;
};
