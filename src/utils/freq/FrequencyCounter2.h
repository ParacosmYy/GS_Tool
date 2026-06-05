/**
 * @file FrequencyCounter2.h
 * @brief 高精度频率计数器 — 多通道独立频率/周期/占空比测量
 *
 * 功能: 实时测量数字信号的频率/周期/占空比/脉宽，
 *       支持多通道并行计数和统计。
 *
 * 协作: SerialTimingAnalyzer(串口时序) / ScopeWidget(示波器测量)
 */
#ifndef FREQUENCYCOUNTER2_H
#define FREQUENCYCOUNTER2_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QElapsedTimer>

class FrequencyCounter2 : public QObject {
    Q_OBJECT

public:
    /** @brief 通道配置 */
    struct ChannelConfig {
        double triggerLevel = 0.5;      ///< 触发电平(归一化)
        double hysteresis = 0.05;       ///< 滞回
        int minPulseWidth = 2;          ///< 最小脉宽(采样点)
    };

    /** @brief 测量结果 */
    struct Measurement {
        double frequency = 0.0;         ///< 频率(Hz)
        double period = 0.0;            ///< 周期(s)
        double dutyCycle = 0.0;         ///< 占空比(%)
        double pulseWidth = 0.0;        ///< 正脉宽(s)
        double riseTime = 0.0;          ///< 上升时间(s)
        int edgeCount = 0;              ///< 边沿计数
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalEdges = 0;                 ///< 累计边沿数
        quint64 totalMeasurements = 0;          ///< 累计测量次数
        QMap<int, quint64> edgesByChannel;      ///< 各通道边沿数
    };

    explicit FrequencyCounter2(QObject* parent = nullptr);

    int addChannel(const ChannelConfig& config);
    void removeChannel(int id);

    /** @brief 输入新采样值 @param channel 通道ID @param value 值 @param timestampUs 时间戳(us) */
    void feed(int channel, double value, qint64 timestampUs);

    /** @brief 获取最新测量 @param channel 通道ID @return 测量结果 */
    Measurement measurement(int channel) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 频率更新 @param channel 通道 @param freq 频率 */
    void frequencyUpdated(int channel, double freq);

    /** @brief 边沿检测 @param channel 通道 @param rising 是否上升沿 */
    void edgeDetected(int channel, bool rising);

private:
    struct ChannelState {
        ChannelConfig config;
        Measurement lastMeasurement;
        double lastValue;
        qint64 lastRisingEdge;
        qint64 lastFallingEdge;
        bool lastHigh;
        int pulseCount;
    };

    QMap<int, ChannelState> m_channels;
    int m_nextId;
    Stats m_stats;
};

#endif // FREQUENCYCOUNTER2_H
