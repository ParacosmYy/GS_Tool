/**
 * @file GoertzelBank.h
 * @brief 多频率Goertzel滤波器组,用于DTMF/DTT等多音检测
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 多频率Goertzel滤波器组
 *
 * 对指定频率组并行执行Goertzel算法,高效检测DTMF、
 * DTT等多音信号。支持滑窗检测、能量阈值和频率置信度。
 */
class GoertzelBank : public QObject
{
    Q_OBJECT

public:
    /** @brief 单频检测结果 */
    struct ToneResult {
        double frequency = 0.0;     ///< 目标频率(Hz)
        double magnitude = 0.0;     ///< 检测幅值
        double energy = 0.0;        ///< 检测能量
        double phase = 0.0;         ///< 相位(rad)
        bool detected = false;      ///< 是否超过阈值
        double confidence = 0.0;    ///< 置信度[0,1]
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalDetections = 0;         ///< 总检测次数
        int totalTonesFound = 0;         ///< 总检测到的音调数
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit GoertzelBank(QObject* parent = nullptr);

    /**
     * @brief 设置检测频率组
     * @param frequencies 目标频率列表(Hz)
     * @param sampleRate 采样率(Hz)
     */
    void setFrequencies(const QVector<double>& frequencies, double sampleRate);

    /**
     * @brief 对信号执行多频率Goertzel检测
     * @param samples 输入采样数据
     * @param sampleRate 采样率(Hz)
     * @param thresholdDb 检测阈值(dB),默认-30
     * @return 各频率检测结果
     */
    QVector<ToneResult> detect(const QVector<double>& samples,
                               double sampleRate, double thresholdDb = -30.0);

    /**
     * @brief 单频Goertzel算法
     * @param samples 输入采样
     * @param targetFreq 目标频率(Hz)
     * @param sampleRate 采样率(Hz)
     * @return {幅值, 相位}
     */
    QPair<double, double> goertzelSingle(const QVector<double>& samples,
                                         double targetFreq, double sampleRate);

    /**
     * @brief DTMF标准频率检测
     * @param samples 输入采样
     * @param sampleRate 采样率(Hz)
     * @return 检测到的DTMF按键字符,空串表示无检测
     */
    QString detectDTMF(const QVector<double>& samples, double sampleRate);

    /**
     * @brief 获取已配置的频率列表
     * @return 频率列表
     */
    QVector<double> configuredFrequencies() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 检测完成信号 */
    void detectionCompleted(int toneCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<double> m_frequencies;  ///< 已配置的目标频率
    double m_sampleRate = 0.0;      ///< 配置的采样率

    ToneResult computeGoertzel(const QVector<double>& samples,
                               double targetFreq, double sampleRate,
                               double thresholdDb) const;
};
