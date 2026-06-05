/**
 * @file GoertzelAlgorithm.h
 * @brief Goertzel算法 — 单频率DFT快速计算
 *
 * 用于检测特定频率的幅度和相位, 无需计算完整FFT。
 * 适用于DTMF检测、音调检测、单频信号分析等场景。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class GoertzelAlgorithm
 * @brief Goertzel算法 — 高效单频DFT计算
 *
 * 复杂度O(N)检测单个频率(vs FFT的O(N log N)计算所有频率)。
 * 支持多频率同时检测、幅度/相位提取、功率谱密度。
 */
class GoertzelAlgorithm : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetections = 0;    ///< 总检测次数
        quint64 totalSamplesProcessed = 0; ///< 总处理样本数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 单频率检测结果 */
    struct FrequencyResult {
        double frequency = 0.0;  ///< 目标频率(Hz)
        double magnitude = 0.0;  ///< 幅度
        double phase = 0.0;      ///< 相位(弧度)
        double power = 0.0;      ///< 功率
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit GoertzelAlgorithm(QObject* parent = nullptr);

    /**
     * @brief 检测单个频率的幅度和相位
     * @param samples 输入采样数据
     * @param targetFreq 目标频率(Hz)
     * @param sampleRate 采样率(Hz)
     * @return 频率检测结果(幅度、相位、功率)
     */
    FrequencyResult detectFrequency(const QVector<double>& samples,
                                     double targetFreq,
                                     double sampleRate) const;

    /**
     * @brief 批量检测多个频率
     * @param samples 输入采样数据
     * @param targetFreqs 目标频率列表
     * @param sampleRate 采样率
     * @return 各频率检测结果列表
     */
    QVector<FrequencyResult> detectMultipleFrequencies(
        const QVector<double>& samples,
        const QVector<double>& targetFreqs,
        double sampleRate) const;

    /**
     * @brief 计算频段功率
     * @param samples 输入采样数据
     * @param freqStart 起始频率
     * @param freqEnd 结束频率
     * @param numBins 频率分bin数
     * @param sampleRate 采样率
     * @return 各bin功率值
     */
    QVector<double> computeBandPower(const QVector<double>& samples,
                                     double freqStart, double freqEnd,
                                     int numBins,
                                     double sampleRate) const;

    /**
     * @brief 检测DTMF按键(双音多频)
     * @param samples 输入采样数据
     * @param sampleRate 采样率
     * @return 检测到的按键字符(空字符串表示未检测到)
     */
    QString detectDTMF(const QVector<double>& samples,
                       double sampleRate) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 频率检测完成 @param freq 目标频率 @param magnitude 幅度 */
    void frequencyDetected(double freq, double magnitude);

private:
    /** @brief Goertzel核心计算: 返回(实部, 虚部) */
    QPair<double, double> goertzelCore(const QVector<double>& samples,
                                       double targetFreq,
                                       double sampleRate) const;

    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
