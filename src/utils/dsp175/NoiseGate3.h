/**
 * @file NoiseGate3.h
 * @brief 噪声门(前瞻缓冲+迟滞+可配置保持时间) — Noise Gate with Lookahead, Hysteresis and Configurable Hold Time
 *
 * 功能: 实现音频噪声门处理器，支持前瞻缓冲降低瞬态失真、
 *       迟滞阈值防止门抖动、可配置攻击/释放/保持时间。
 *
 * 协作: Compressor(压缩器) / Expander(扩展器) / Limiter(限幅器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 噪声门处理器(前瞻+迟滞)
 */
class NoiseGate3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;          ///< 累计处理采样数
        quint64 gatedSamples = 0;          ///< 被门控的采样数
        double openRatio = 0.0;            ///< 门开启比例
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit NoiseGate3(QObject *parent = nullptr);
    ~NoiseGate3() override;

    /** @brief 设置阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置迟滞量(dB) */
    void setHysteresis(double hysteresisDb);

    /** @brief 设置攻击时间(ms) */
    void setAttack(double attackMs);

    /** @brief 设置释放时间(ms) */
    void setRelease(double releaseMs);

    /** @brief 设置保持时间(ms) */
    void setHold(double holdMs);

    /** @brief 设置前瞻采样数 */
    void setLookaheadSamples(int samples);

    /** @brief 设置采样率 */
    void setSampleRate(double rate);

    /**
     * @brief 处理音频缓冲区
     * @param input 输入采样
     * @return 门控后的输出
     */
    QVector<double> process(const QVector<double>& input);

    /** @brief 单采样点处理(无前瞻) */
    double processOne(double sample);

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double openRatio);
    void gateStateChanged(bool open);

private:
    /** @brief 计算RMS包络 */
    double computeEnvelope(double sample);

    /** @brief dB转线性 */
    static double dbToLinear(double db);

    /** @brief 线性转dB */
    static double linearToDb(double linear);

    /** @brief 平滑增益过渡 */
    double smoothGain(double target, double coeff);

    double m_sampleRate = 44100.0;
    double m_thresholdDb = -40.0;
    double m_hysteresisDb = 6.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
    double m_holdMs = 50.0;
    int m_lookahead = 64;

    /* Runtime state */
    double m_envelope = 0.0;        ///< 当前包络值
    double m_gain = 0.0;            ///< 当前增益(0~1)
    bool m_gateOpen = false;        ///< 门是否开启
    int m_holdCounter = 0;          ///< 保持计数器
    double m_attackCoeff = 0.0;     ///< 攻击系数
    double m_releaseCoeff = 0.0;    ///< 释放系数
    int m_holdSamples = 0;          ///< 保持采样数

    QVector<double> m_lookaheadBuf; ///< 前瞻环形缓冲
    int m_laWritePos = 0;           ///< 前瞻写位置
    int m_laReadPos = 0;            ///< 前瞻读位置
    bool m_laFilled = false;        ///< 前瞻缓冲已满

    Stats m_stats;
    double m_timeSum = 0.0;
};
