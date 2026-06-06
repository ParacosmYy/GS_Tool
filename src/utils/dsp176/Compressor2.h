/**
 * @file Compressor2.h
 * @brief 动态范围压缩器(软/硬拐点+侧链+并行NY压缩) — Dynamic Range Compressor with Soft/Hard Knee, Sidechain and Parallel Compression
 *
 * 功能: 实现动态范围压缩器，支持软/硬拐点曲线、外部侧链输入、
 *       并行(New York)压缩模式和RMS/Peak检测。
 *
 * 协作: WindowFunction(窗函数) / FftEngine(FFT) / Compressor(基础压缩)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态范围压缩器(侧链+NY并行)
 */
class Compressor2 : public QObject {
    Q_OBJECT

public:
    /** @brief 检测模式 */
    enum DetectionMode { Peak = 0, RMS = 1 };

    /** @brief 拐点类型 */
    enum KneeType { HardKnee = 0, SoftKnee = 1 };

    /** @brief 压缩参数 */
    struct Parameters {
        double threshold = -20.0;    ///< 阈值(dB)
        double ratio = 4.0;          ///< 压缩比
        double attack = 10.0;        ///< 启动时间(ms)
        double release = 100.0;      ///< 释放时间(ms)
        double kneeWidth = 6.0;      ///< 拐点宽度(dB)
        double makeupGain = 0.0;     ///< 补偿增益(dB)
        double dryMix = 0.0;         ///< 干信号比例(NY压缩)
        DetectionMode detection = Peak;
        KneeType knee = SoftKnee;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProcessed = 0;        ///< 累计采样数
        double peakGainReduction = 0.0;    ///< 峰值增益衰减(dB)
        double avgGainReduction = 0.0;     ///< 平均增益衰减(dB)
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit Compressor2(QObject *parent = nullptr);
    ~Compressor2() override;

    void setParameters(const Parameters& params);
    void setSampleRate(double rate);

    /**
     * @brief 处理单通道信号
     * @param input 输入采样
     * @return 压缩后采样
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 侧链压缩: 用侧链信号控制增益
     * @param input 输入信号
     * @param sidechain 侧链信号(同长度)
     * @return 压缩后信号
     */
    QVector<double> processSidechain(const QVector<double>& input,
                                     const QVector<double>& sidechain);

    /** @brief 获取增益包络(dB) */
    QVector<double> gainEnvelope() const;

    /** @brief 计算静态增益曲线(dB) */
    QVector<QVector<double>> transferCurve(int points = 256) const;

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double gainReduction);

private:
    /** @brief 计算增益衰减(dB) */
    double computeGainReduction(double inputDb) const;

    /** @brief 转dB */
    static double toDb(double linear);
    /** @brief 从dB转线性 */
    static double fromDb(double db);

    Parameters m_params;
    double m_sampleRate = 44100.0;

    /* Envelope follower state */
    double m_envelope = 0.0;           ///< 当前增益包络(dB)
    double m_attackCoeff = 0.0;        ///< 启动系数
    double m_releaseCoeff = 0.0;       ///< 释放系数

    QVector<double> m_gainEnv;         ///< 增益包络历史

    Stats m_stats;
    double m_timeSum = 0.0;
};
