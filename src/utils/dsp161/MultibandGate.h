/**
 * @file MultibandGate.h
 * @brief 6段噪声门 — 6-Band Noise Gate with Independent Threshold/Hysteresis
 *
 * 功能: 将音频信号分为6个频段，每个频段独立配置门限阈值和迟滞区间，
 *       支持attack/release/hold时间参数。适用于多频段动态处理和噪声抑制。
 *
 * 协作: WienerFilter(维纳滤波降噪) / ConstantQTransform(频域分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 6段噪声门处理器
 */
class MultibandGate : public QObject {
    Q_OBJECT

public:
    /** @brief 单频段参数 */
    struct BandConfig {
        double lowFreq = 0.0;       ///< 频段下限(Hz)
        double highFreq = 0.0;      ///< 频段上限(Hz)
        double threshold = -40.0;   ///< 门限阈值(dB)
        double hysteresis = 6.0;    ///< 迟滞区间(dB)
        double attack = 1.0;        ///< 启动时间(ms)
        double release = 50.0;      ///< 释放时间(ms)
        double hold = 10.0;         ///< 保持时间(ms)
        double range = -80.0;       ///< 最大衰减量(dB)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;            ///< 累计处理帧数
        quint64 totalGateEvents = 0;        ///< 累计门控事件数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    static const int BAND_COUNT = 6;

    explicit MultibandGate(QObject* parent = nullptr);

    /**
     * @brief 设置采样率
     * @param rate 采样率(Hz)
     */
    void setSampleRate(double rate);

    /**
     * @brief 设置指定频段参数
     * @param band 频段索引(0-5)
     * @param config 频段配置
     */
    void setBandConfig(int band, const BandConfig& config);

    /**
     * @brief 使用默认6段频率划分初始化
     */
    void initDefaultBands();

    /**
     * @brief 处理一帧音频数据
     * @param input 输入采样数据
     * @return 门控后的采样数据
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 重置内部状态
     */
    void reset();

    /** @brief 获取频段配置 */
    QVector<BandConfig> bandConfigs() const { return m_bands; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param frameSize 帧大小 */
    void frameProcessed(int frameSize);

private:
    /** @brief 频段内部状态 */
    struct BandState {
        bool gateOpen = false;          ///< 门是否打开
        double envelopeLevel = 0.0;     ///< 包络电平(dB)
        double gain = 1.0;             ///< 当前增益
        double holdTimer = 0.0;        ///< 保持计时器(ms)
        QVector<double> biquadA;       ///< 滤波器系数a
        QVector<double> biquadB;       ///< 滤波器系数b
        double filterState[4] = {};    ///< 滤波器状态[x1,x2,y1,y2]
    };

    /** @brief 设计Linkwitz-Riley 4阶分频滤波器 */
    void designCrossoverFilter(int band, double lowFreq, double highFreq);

    /** @brief 应用带通滤波器 */
    double applyFilter(int band, double sample);

    /** @brief 计算dB电平 */
    static double toDB(double linear) { return 20.0 * qLn(qMax(1e-10, linear)) / qLn(10.0); }
    /** @brief dB转线性 */
    static double fromDB(double db) { return qPow(10.0, db / 20.0); }

    double m_sampleRate = 44100.0;
    QVector<BandConfig> m_bands;
    QVector<BandState> m_states;

    Stats m_stats;
    double m_timeSum = 0.0;
};
