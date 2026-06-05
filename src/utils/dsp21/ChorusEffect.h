/**
 * @file ChorusEffect.h
 * @brief 合唱/镶边效果器 — 多调制延迟线/LFO/立体声展宽
 *
 * 功能: 实现合唱和镶边音频效果，支持多条调制延迟线，
 *       LFO正弦/三角波调制，立体声展宽，反馈控制和干湿混合。
 *
 * 协作: SignalGenerator(信号生成) / WaveformGenerator(波形显示)
 */
#ifndef CHORUSEFFECT_H
#define CHORUSEFFECT_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @brief 合唱/镶边效果处理器
 */
class ChorusEffect : public QObject {
    Q_OBJECT

public:
    /** @brief LFO波形类型 */
    enum class LfoWaveform {
        Sine,       ///< 正弦波(平滑合唱)
        Triangle    ///< 三角波(明亮镶边)
    };
    Q_ENUM(LfoWaveform)

    /** @brief 效果参数 */
    struct ChorusParams {
        double rate = 1.5;          ///< LFO速率(Hz)
        double depth = 0.005;       ///< 调制深度(秒)
        double baseDelay = 0.025;   ///< 基础延迟(秒)
        double feedback = 0.3;      ///< 反馈增益(0-0.95)
        double mix = 0.5;           ///< 干湿混合(0-1)
        double stereoSpread = 0.5;  ///< 立体声展宽(0-1)
        int voiceCount = 3;         ///< 延迟线数量(1-8)
        LfoWaveform waveform = LfoWaveform::Sine;
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样数
        quint64 totalBlocksProcessed = 0;   ///< 累计处理块数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  peakAmplitude = 0.0;        ///< 峰值输出幅度
    };

    explicit ChorusEffect(QObject* parent = nullptr);

    /** @brief 设置效果参数 @param params 参数结构 */
    void setParams(const ChorusParams& params);

    /** @brief 初始化处理器 @param sampleRate 采样率 @param maxDelay 最大延迟(秒) */
    void initialize(double sampleRate, double maxDelay = 0.1);

    /** @brief 处理单声道采样 @param input 输入采样 @return 处理后采样 */
    double processSample(double input);

    /** @brief 处理单声道块 @param input 输入块 @return 处理后块 */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief 处理立体声块 @param left 左声道 @param right 右声道 @return (左,右) */
    QPair<QVector<double>, QVector<double>> processStereo(
        const QVector<double>& left, const QVector<double>& right);

    /** @brief 重置延迟线和LFO相位 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 块处理完成 @param sampleCount 采样数 */
    void blockProcessed(int sampleCount);

private:
    double lfoValue(int voiceIndex) const;
    double readDelay(int voiceIndex, double delaySamples) const;
    void writeDelay(int voiceIndex, double sample);

    ChorusParams m_params;          ///< 效果参数
    double m_sampleRate;            ///< 采样率

    /* 每条延迟线的环形缓冲区 */
    QVector<QVector<double>> m_delayBuffers;
    QVector<int> m_writePos;        ///< 每条线的写指针
    int m_bufferSize;               ///< 缓冲区大小(采样数)

    double m_lfoPhase;              ///< LFO当前相位
    bool m_initialized;             ///< 是否已初始化

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

#endif // CHORUSEFFECT_H
