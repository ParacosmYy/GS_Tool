/**
 * @file DynamicEQ.h
 * @brief 动态均衡器(电平相关增益+侧链检测) — Dynamic Equalizer with Level-Dependent Gain and Sidechain Detection
 *
 * 功能: 实现动态均衡器，根据输入电平或侧链信号动态调整频段增益，
 *       支持多频段滤波器组、RMS/Peak检测和压缩/扩展模式。
 *
 * 协作: BiquadFilter(双二阶滤波器) / Compressor2(动态压缩) / SpectrumAnalyzer(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态均衡器
 */
class DynamicEQ : public QObject {
    Q_OBJECT

public:
    /** @brief 检测模式 */
    enum DetectMode { RMS, Peak };

    /** @brief 动作模式 */
    enum ActionMode { Compress, Expand };

    /** @brief 频段参数 */
    struct BandParams {
        double freq = 1000.0;       ///< 中心频率(Hz)
        double gain = 0.0;          ///< 基础增益(dB)
        double q = 1.0;             ///< Q值
        double threshold = -20.0;   ///< 阈值(dB)
        double ratio = 4.0;         ///< 压缩/扩展比
        double attack = 10.0;       ///< 启动时间(ms)
        double release = 100.0;     ///< 释放时间(ms)
        double range = 20.0;        ///< 最大增益调节范围(dB)
        ActionMode mode = Compress; ///< 动作模式
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;         ///< 累计处理样本数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int bandCount = 0;                ///< 频段数
    };

    explicit DynamicEQ(double sampleRate = 44100.0, QObject *parent = nullptr);
    ~DynamicEQ() override;

    void setSampleRate(double sr);
    void setDetectMode(DetectMode mode);

    /** @brief 设置频段参数 */
    void setBand(int index, const BandParams& params);

    /** @brief 添加频段 */
    int addBand(const BandParams& params);

    /** @brief 移除频段 */
    void removeBand(int index);

    /**
     * @brief 处理音频帧
     * @param input 输入采样
     * @return 输出采样
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 带侧链输入处理
     * @param input 主输入
     * @param sidechain 侧链输入
     * @return 输出采样
     */
    QVector<double> processWithSidechain(const QVector<double>& input,
                                         const QVector<double>& sidechain);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param samples 样本数 */
    void processingCompleted(int samples);

private:
    /** @brief 二阶IIR滤波器节 */
    struct Biquad {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
        double z1 = 0.0, z2 = 0.0;
        double process(double x) {
            double y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };

    /** @brief 频段状态 */
    struct BandState {
        BandParams params;
        Biquad filter;
        double envLevel = 0.0;   ///< 包络电平(dB)
        double dynGain = 0.0;    ///< 当前动态增益(dB)
    };

    /** @brief 设计峰化滤波器 */
    void designPeakFilter(Biquad& bq, double freq, double gainDb, double q) const;

    /** @brief 计算压缩/扩展增益 */
    double computeGain(double inputDb, const BandParams& bp) const;

    /** @brief 更新包络 */
    double updateEnvelope(double current, double target,
                          double coeffAttack, double coeffRelease) const;

    double m_sampleRate;
    DetectMode m_detect = RMS;

    QVector<BandState> m_bands;

    Stats m_stats;
    double m_timeSum = 0.0;
};
