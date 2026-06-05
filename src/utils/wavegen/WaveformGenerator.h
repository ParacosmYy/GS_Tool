/**
 * @file WaveformGenerator.h
 * @brief 流式波形发生器 -- 支持调制/扫频/自定义波形/多格式量化的流式信号源
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 与 utils/waveform/WaveformGenerator 不同，本类专注于流式输出：
 * 按可配置块大小持续产生波形数据，适合串口发送或 DAC 驱动场景。
 * 支持 AM/FM 调制、相位连续频率扫掠、自定义波形插值。
 */

#ifndef WAVEGEN_WAVEFORMGENERATOR_H
#define WAVEGEN_WAVEFORMGENERATOR_H

#include <QByteArray>
#include <QObject>
#include <QTimer>
#include <QVector>

#include "utils/wavegen/WaveGenTypes.h"

/**
 * @class WaveformGenerator
 * @brief 流式波形发生器，按块输出量化后的波形字节流
 *
 * 核心流程:
 *   1. setParams() 配置波形参数
 *   2. startStreaming() 启动定时器，按 chunkSize 持续生成
 *   3. dataGenerated(QByteArray) 信号输出量化后的字节流
 *   4. stopStreaming() 停止
 *
 * 相位在连续生成中自动累积，保证波形连续无跳变。
 */
class WaveformGenerator : public QObject {
    Q_OBJECT

public:
    /** @brief 构造波形发生器 @param parent 父对象 */
    explicit WaveformGenerator(QObject *parent = nullptr);

    /** @brief 析构，停止流式输出 */
    ~WaveformGenerator() override;

    // ── 参数配置 ──

    /** @brief 设置波形参数 @param params 参数结构体 */
    void setParams(const WaveGen::WaveGenParams &params);

    /** @brief 获取当前参数 @return 参数结构体 */
    WaveGen::WaveGenParams params() const;

    /** @brief 设置流式输出块大小（采样点数） @param size 每块采样数 */
    void setChunkSize(int size);

    /** @brief 获取块大小 @return 每块采样数 */
    int chunkSize() const;

    // ── 流式输出控制 ──

    /** @brief 启动流式输出 @param intervalMs 定时器间隔(ms), 0=最快 */
    void startStreaming(int intervalMs = 10);

    /** @brief 停止流式输出 */
    void stopStreaming();

    /** @brief 是否正在流式输出 @return true=流式进行中 */
    bool isStreaming() const;

    // ── 单次生成 ──

    /** @brief 按当前参数生成指定采样数的原始波形 @param count 采样点数 @return 采样值向量 */
    QVector<double> generateRaw(int count);

    /** @brief 将原始数据量化为字节流 @param raw 原始采样值 @return 量化后的字节流 */
    QByteArray quantize(const QVector<double> &raw) const;

    // ── 统计 ──

    /** @brief 获取统计快照 @return 统计结构体 */
    WaveGen::Stats stats() const;

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 流式数据就绪 @param data 量化后的字节流 */
    void dataGenerated(const QByteArray &data);

    /** @brief 参数已变更 */
    void paramsChanged();

private slots:
    void onStreamingTick();  ///< 流式定时器回调

private:
    double computeSample(int index);            ///< 计算单个采样值
    double applyModulation(double sample, double t);  ///< 应用AM/FM调制
    QByteArray toBytes(const QVector<double> &raw) const; ///< 量化+字节序转换

    WaveGen::WaveGenParams m_params;    ///< 当前参数
    int m_chunkSize           = 1024;   ///< 每块采样数
    double m_phaseAccumulator = 0.0;    ///< 相位累积器(弧度)
    int m_sampleIndex         = 0;      ///< 全局采样索引
    bool m_streaming          = false;  ///< 流式输出状态
    QTimer *m_timer;                    ///< 流式定时器
    WaveGen::Stats m_stats;             ///< 统计计数器
};

#endif // WAVEGEN_WAVEFORMGENERATOR_H
