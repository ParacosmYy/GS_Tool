/**
 * @file LogicSampler.h
 * @brief 数字信号采集引擎 -- 多通道数字信号采样与触发检测
 *
 * LogicSampler 负责:
 *   - 多通道数字信号采集配置(通道数/采样率/采样深度)
 *   - 原始数据馈入与解析(每字节打包的通道状态)
 *   - 触发条件检测(通道电平匹配)
 *   - 预触发环形缓冲区(保留触发前的历史数据)
 *   - 采集会话生命周期管理(start/stop)
 *   - 统计信息追踪(样本数/字节数/触发次数/会话数)
 */
#ifndef LOGIC_SAMPLER_H
#define LOGIC_SAMPLER_H

#include <QObject>
#include <QVector>
#include "protocol/logic/LogicTypes.h"

/**
 * @brief 数字信号采集引擎
 *
 * 通过 configure() 设置采集参数，start()/stop() 控制采集会话，
 * feedData() 馈入硬件原始数据。支持触发条件检测和预触发缓冲。
 */
class LogicSampler : public QObject {
    Q_OBJECT

public:
    /** @brief 构造数字信号采集引擎 @param parent 父QObject指针 */
    explicit LogicSampler(QObject* parent = nullptr);

    /** @brief 配置采集参数 @param channelCount 通道数量(1~8) @param sampleRateHz 采样率(Hz) @param sampleCount 目标采样数量 */
    void configure(int channelCount, double sampleRateHz, quint64 sampleCount);

    /** @brief 启动采集会话 @return 配置有效返回true */
    bool start();

    /** @brief 停止当前采集会话 */
    void stop();

    /** @brief 查询采集是否正在进行 @return true=正在采集 */
    bool isRunning() const;

    /**
     * @brief 馈入原始采样数据
     * @param rawSamples 原始字节流，每字节低N位对应N个通道的电平状态
     *
     * 数据格式: 每字节为一个采样点，bit[0]~bit[channelCount-1] 为通道电平。
     * 时间戳按采样率间隔自动递增。
     */
    void feedData(const QByteArray& rawSamples);

    /** @brief 获取已采集的样本列表 @return LogicSample向量 */
    QVector<LogicSample> samples() const;

    /** @brief 获取配置的通道数量 */
    int channelCount() const;

    /** @brief 获取配置的采样率(Hz) */
    double sampleRate() const;

    /** @brief 获取配置的目标采样数量 */
    quint64 sampleCount() const;

    /** @brief 获取已采集的样本数量 */
    quint64 capturedSamples() const;

    /** @brief 设置触发条件 @param trigger 触发配置 */
    void setTrigger(const LogicTrigger& trigger);

    /** @brief 获取当前触发条件 @return 触发配置 */
    LogicTrigger trigger() const;

    // ── 统计信息 ──
    /** @brief 获取累计处理的样本总数 */
    quint64 totalSamplesProcessed() const;
    /** @brief 获取累计接收的原始字节数 */
    quint64 totalBytesReceived() const;
    /** @brief 获取累计触发次数 */
    quint64 totalTriggersFired() const;
    /** @brief 获取累计采集会话数 */
    quint64 totalCaptureSessions() const;
    /** @brief 获取最近一次采集的持续时间(ms) */
    double captureDurationMs() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 采集会话已启动 */
    void captureStarted();

    /** @brief 采集会话已停止 */
    void captureStopped();

    /** @brief 触发条件已满足 @param position 触发点在样本缓冲区中的位置 */
    void triggerFired(quint64 position);

    /** @brief 新样本数据可用 @param samples 新增的样本列表 */
    void dataAvailable(const QVector<LogicSample>& samples);

private:
    /** @brief 检测触发条件 @param sample 当前采样点 @return true=触发匹配 */
    bool checkTrigger(const LogicSample& sample) const;

    /** @brief 将原始字节解析为LogicSample序列 @param rawBytes 原始字节流 @return 解析后的样本列表 */
    QVector<LogicSample> parseRawData(const QByteArray& rawBytes) const;

    // ── 采集配置 ──
    int     m_channelCount  = 1;           ///< 通道数量
    double  m_sampleRateHz  = 1e6;         ///< 采样率(Hz)
    quint64 m_sampleCount   = 1000000;     ///< 目标采样数量
    // ── 采集状态 ──
    bool    m_running       = false;       ///< 是否正在采集
    quint64 m_nextTimestamp = 0;           ///< 下一个样本的时间戳(ns)
    quint64 m_captureStartNs = 0;          ///< 本次采集开始时间(ns)
    // ── 触发 ──
    LogicTrigger m_trigger;                ///< 触发条件
    bool    m_triggerArmed  = false;       ///< 触发是否已激活
    bool    m_triggerMatched = false;      ///< 触发是否已匹配
    // ── 数据缓冲 ──
    QVector<LogicSample> m_samples;        ///< 已采集样本缓冲区
    QVector<LogicSample> m_preTriggerBuf;  ///< 预触发环形缓冲区
    int     m_preTriggerIdx = 0;           ///< 环形缓冲区写入位置
    // ── 统计计数器 ──
    quint64 m_totalSamplesProcessed = 0;   ///< 累计处理样本数
    quint64 m_totalBytesReceived    = 0;   ///< 累计接收字节数
    quint64 m_totalTriggersFired    = 0;   ///< 累计触发次数
    quint64 m_totalCaptureSessions  = 0;   ///< 累计采集会话数
    double  m_captureDurationMs     = 0.0; ///< 最近采集持续时间(ms)
};

#endif // LOGIC_SAMPLER_H
