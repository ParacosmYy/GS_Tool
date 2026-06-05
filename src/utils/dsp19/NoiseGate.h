/**
 * @file NoiseGate.h
 * @brief 噪声门处理器 — 阈值检测/包络控制/侧链滤波
 *
 * 功能: 实现专业噪声门处理，支持阈值检测、攻击/释放/保持包络、
 *       滞回比较、侧链滤波和前瞻缓冲。
 *
 * 协作: DigitalFilter(滤波) / SpectrumAnalyzer(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 噪声门处理器 — 实时信号噪声抑制
 */
class NoiseGate : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样数
        quint64 totalBlocksProcessed = 0;   ///< 累计处理数据块数
        quint64 totalSamplesGated = 0;      ///< 累计被门控的采样数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  gateRatio = 0.0;            ///< 门控比例(0-1)
    };

    explicit NoiseGate(QObject* parent = nullptr);

    /** @brief 设置门控阈值(dB) @param threshold 阈值 */
    void setThreshold(double threshold);

    /** @brief 设置攻击时间(ms) @param attack 攻击时间 */
    void setAttack(double attack);

    /** @brief 设置释放时间(ms) @param release 释放时间 */
    void setRelease(double release);

    /** @brief 设置保持时间(ms) @param hold 保持时间 */
    void setHold(double hold);

    /** @brief 设置滞回宽度(dB) @param hysteresis 滞回宽度 */
    void setHysteresis(double hysteresis);

    /** @brief 设置侧链低截止频率(Hz) @param freq 频率 */
    void setSidechainLowCut(double freq);

    /** @brief 设置前瞻采样数 @param samples 采样数 */
    void setLookahead(int samples);

    /**
     * @brief 处理音频块
     * @param input 输入采样数据
     * @param sampleRate 采样率
     * @return 处理后的采样数据
     */
    QVector<double> process(const QVector<double>& input, double sampleRate);

    /**
     * @brief 检测信号是否超过阈值
     * @param sample 输入采样
     * @return true=超过阈值
     */
    bool detectThreshold(double sample);

    /**
     * @brief 计算当前包络增益
     * @return 增益值(0-1)
     */
    double currentEnvelope() const;

    /**
     * @brief 应用侧链高通滤波
     * @param samples 输入采样
     * @param sampleRate 采样率
     * @return 滤波后采样
     */
    QVector<double> applySidechainFilter(const QVector<double>& samples,
                                         double sampleRate);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 门状态变化 @param isOpen true=门打开 */
    void gateStateChanged(bool isOpen);

    /** @brief 处理完成 @param totalSamples 总采样数 @param gatedSamples 被门控采样数 */
    void blockProcessed(int totalSamples, int gatedSamples);

private:
    void updateEnvelope(bool aboveThreshold, double sampleRate);
    double simpleHighpass(double sample);

    double m_threshold;             ///< 门控阈值(线性值)
    double m_attackCoeff;           ///< 攻击系数
    double m_releaseCoeff;          ///< 释放系数
    double m_holdTime;              ///< 保持时间(采样数)
    double m_hysteresis;            ///< 滞回宽度(线性值)
    double m_sidechainFreq;         ///< 侧链低截止频率
    int m_lookahead;                ///< 前瞻采样数

    double m_envelope;              ///< 当前包络值
    bool m_gateOpen;                ///< 门状态
    int m_holdCounter;              ///< 保持计数器
    double m_prevSidechain;         ///< 侧链滤波器状态

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
