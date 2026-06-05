/**
 * @file GoertzelBank2.h
 * @brief 多频率Goertzel滤波器组 — 特定频率检测/DTMF
 *
 * 功能: 不需要完整FFT即可检测特定频率成分，支持并行Goertzel
 *       滤波器组、DTMF拨号音检测、能量累积与频率判定。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / NoiseGate(信号预处理)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>

/**
 * @brief 多频率Goertzel滤波器组 — 定点频率检测引擎
 */
class GoertzelBank2 : public QObject {
    Q_OBJECT

public:
    /** @brief 单频率检测结果 */
    struct FrequencyResult {
        double frequency = 0.0;    ///< 目标频率(Hz)
        double magnitude = 0.0;    ///< 检测幅度
        double power = 0.0;        ///< 功率值
        bool   detected = false;   ///< 是否超过检测阈值
    };

    /** @brief DTMF检测结果 */
    struct DtmfResult {
        QChar digit;               ///< 检测到的DTMF字符
        double confidence = 0.0;   ///< 置信度(0-1)
        double rowFreq = 0.0;      ///< 行频率
        double colFreq = 0.0;      ///< 列频率
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalBlocksProcessed = 0;   ///< 累计处理数据块数
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样数
        quint64 totalDetections = 0;        ///< 累计检测到的频率数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  avgEnergyLevel = 0.0;       ///< 平均能量水平
    };

    explicit GoertzelBank2(QObject* parent = nullptr);

    /** @brief 设置目标频率列表 @param frequencies 频率(Hz) */
    void setTargetFrequencies(const QVector<double>& frequencies);

    /** @brief 设置检测阈值(dB) @param threshold 阈值 */
    void setDetectionThreshold(double threshold);

    /**
     * @brief 处理数据块，检测所有目标频率
     * @param samples 输入采样数据
     * @param sampleRate 采样率
     * @return 各频率检测结果
     */
    QList<FrequencyResult> processBank(const QVector<double>& samples,
                                       double sampleRate);

    /**
     * @brief 单个Goertzel滤波器
     * @param samples 输入采样
     * @param targetFreq 目标频率
     * @param sampleRate 采样率
     * @return (幅度, 功率)
     */
    QPair<double, double> goertzelFilter(const QVector<double>& samples,
                                         double targetFreq,
                                         double sampleRate);

    /**
     * @brief DTMF拨号音检测
     * @param samples 输入采样
     * @param sampleRate 采样率
     * @return DTMF检测结果
     */
    DtmfResult detectDTMF(const QVector<double>& samples,
                          double sampleRate);

    /**
     * @brief 计算信号总能量
     * @param samples 采样数据
     * @return 能量值
     */
    double computeEnergy(const QVector<double>& samples) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 频率检测完成 @param results 检测结果列表 */
    void frequenciesDetected(const QList<FrequencyResult>& results);

    /** @brief DTMF检测完成 @param digit 检测到的字符 @param confidence 置信度 */
    void dtmfDetected(QChar digit, double confidence);

private:
    double computeCoefficient(double targetFreq, double sampleRate) const;

    QVector<double> m_targetFreqs;      ///< 目标频率列表
    double m_threshold;                  ///< 检测阈值(线性值)
    QMap<QPair<double, double>, QChar> m_dtmfMap; ///< DTMF频率映射表

    Stats m_stats;
    double m_timeSum = 0.0;     ///< 处理时间累加器
};
