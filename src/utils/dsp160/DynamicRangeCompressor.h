/**
 * @file DynamicRangeCompressor.h
 * @brief 动态范围压缩器 — Dynamic Range Compressor
 *
 * 功能: 支持attack/release时间、阈值/比率/软拐点参数配置，
 *       增益平滑与RMS/峰值检测模式。适用于音频动态范围控制。
 *
 * 协作: Limiter(限制器) / Gate(噪声门) / MultibandCompressor(多段压缩)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态范围压缩器，支持可配置的attack/release/threshold/ratio/knee
 */
class DynamicRangeCompressor : public QObject {
    Q_OBJECT

public:
    /** @brief 检测模式 */
    enum class DetectionMode {
        RMS,    ///< 均方根检测
        Peak    ///< 峰值检测
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;           ///< 累计处理采样点数
        quint64 totalGainReductions = 0;    ///< 累计增益降低次数
        double avgReductionDb = 0.0;        ///< 平均增益降低量(dB)
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit DynamicRangeCompressor(QObject* parent = nullptr);

    /**
     * @brief 设置attack时间
     * @param ms Attack时间(毫秒)，范围0.1~100
     */
    void setAttack(double ms);

    /**
     * @brief 设置release时间
     * @param ms Release时间(毫秒)，范围10~3000
     */
    void setRelease(double ms);

    /**
     * @brief 设置压缩阈值
     * @param db 阈值(dB)，范围-60~0
     */
    void setThreshold(double db);

    /**
     * @brief 设置压缩比率
     * @param ratio 压缩比，范围1:1~20:1
     */
    void setRatio(double ratio);

    /**
     * @brief 设置软拐点宽度
     * @param db 拐点宽度(dB)，0=硬拐点，范围0~24
     */
    void setKnee(double db);

    /**
     * @brief 设置检测模式
     * @param mode RMS或Peak检测
     */
    void setDetectionMode(DetectionMode mode);

    /**
     * @brief 处理单帧音频数据
     * @param input 输入采样点序列
     * @return 压缩后的采样点序列
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 重置内部状态(增益平滑)
     */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param samples 帧长度 @param reductionDb 平均增益降低 */
    void frameProcessed(int samples, double reductionDb);

private:
    /** @brief 计算压缩增益(dB) */
    double computeGainReduction(double inputLevelDb) const;

    /** @brief 增益平滑(attack/release包络跟随) */
    double smoothGain(double targetGainDb, double currentGainDb) const;

    double m_attackMs = 10.0;
    double m_releaseMs = 100.0;
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_kneeDb = 6.0;
    DetectionMode m_detectionMode = DetectionMode::RMS;

    double m_currentGainDb = 0.0;     ///< 当前平滑增益
    double m_sampleRate = 44100.0;    ///< 采样率
    double m_rmsWindow = 0.0;         ///< RMS平滑窗口

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_reductionSum = 0.0;
};
