/**
 * @file StereoProcessor.h
 * @brief 立体声处理器 — Mid/Side编解码/宽度控制/声像/哈斯效应/相关性表
 *
 * 功能: 实现完整的立体声音频处理链，包括Mid/Side编码解码、
 *       立体声宽度调节、平衡声像控制、哈斯效应延迟、
 *       以及立体声相关性表。
 *
 * 协作: SignalDecomposer(信号分解) / WaveformGenerator(波形生成)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 立体声处理器 — 立体声场控制与Mid/Side处理
 */
class StereoProcessor : public QObject {
    Q_OBJECT

public:
    /** @brief 立体声处理结果 */
    struct StereoFrame {
        QVector<double> left;       ///< 左声道输出
        QVector<double> right;      ///< 右声道输出
        QVector<double> mid;        ///< Mid分量
        QVector<double> side;       ///< Side分量
    };

    /** @brief 相关性表 */
    struct CorrelationMeter {
        double currentCorrelation = 0.0;   ///< 当前相关系数[-1,1]
        double peakCorrelation = 0.0;      ///< 峰值相关
        double rmsLeft = 0.0;             ///< 左声道RMS
        double rmsRight = 0.0;            ///< 右声道RMS
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalFramesProcessed = 0;     ///< 累计处理帧数
        quint64 totalSamplesProcessed = 0;    ///< 累计处理采样数
        double  avgProcessingTimeMs = 0.0;    ///< 平均处理时间(ms)
        double  avgCorrelation = 0.0;        ///< 平均相关系数
    };

    explicit StereoProcessor(QObject* parent = nullptr);

    /** @brief 设置立体声宽度 @param width 宽度[0.0,2.0], 1.0=原始 */
    void setWidth(double width);

    /** @brief 设置平衡声像 @param pan 声像[-1.0,1.0], 0=居中 */
    void setPan(double pan);

    /** @brief 设置哈斯效应延迟 @param delaySamples 延迟采样数 */
    void setHaasDelay(int delaySamples);

    /** @brief 设置采样率 @param sampleRate 采样率 */
    void setSampleRate(double sampleRate);

    /** @brief 处理立体声帧 @param left 左声道输入 @param right 右声道输入 @return 处理后的立体声帧 */
    StereoFrame process(const QVector<double>& left,
                        const QVector<double>& right);

    /** @brief 编码Mid/Side @param left 左声道 @param right 右声道 @return (mid, side) */
    QPair<QVector<double>, QVector<double>> encodeMidSide(
        const QVector<double>& left, const QVector<double>& right);

    /** @brief 解码Mid/Side @param mid Mid分量 @param side Side分量 @return (left, right) */
    QPair<QVector<double>, QVector<double>> decodeMidSide(
        const QVector<double>& mid, const QVector<double>& side);

    /** @brief 获取相关性表 @return 当前相关性数据 */
    CorrelationMeter correlationMeter() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param sampleCount 采样数 @param correlation 当前相关系数 */
    void frameProcessed(int sampleCount, double correlation);

private:
    double computeCorrelation(const QVector<double>& left,
                              const QVector<double>& right) const;
    double computeRms(const QVector<double>& data) const;

    double m_width;                ///< 立体声宽度
    double m_pan;                  ///< 声像
    int m_haasDelay;              ///< 哈斯延迟采样数
    double m_sampleRate;          ///< 采样率
    CorrelationMeter m_corrMeter; ///< 相关性表

    /** @brief 攻击/释放平滑滤波器状态 */
    double m_smoothCorrelation;   ///< 平滑后的相关系数
    double m_attackCoeff;         ///< 攻击系数
    double m_releaseCoeff;        ///< 释放系数

    /** @brief 哈斯延迟线 */
    QVector<double> m_delayLine;  ///< 延迟线缓冲
    int m_delayWritePos;          ///< 延迟线写入位置

    Stats m_stats;
    double m_timeSum = 0.0;       ///< 处理时间累加器
};
