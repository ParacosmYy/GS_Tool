#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态范围压缩器实现 (版本6)
 *
 * 提供音频/信号动态范围压缩，支持多种压缩曲线和启动/释放时间控制。
 */
class Compressor6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalCompressionRuns = 0;   ///< 总压缩处理次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double peakReductionDb = 0.0;   ///< 峰值增益衰减(dB)
    };

    explicit Compressor6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频数据压缩
     * @param samples 输入采样数据
     * @return 压缩后的采样数据
     */
    QVector<double> process(const QVector<double>& samples);

    /**
     * @brief 设置压缩阈值
     * @param thresholdDb 阈值(dB)
     * @param ratio 压缩比
     * @param kneeDb 软拐点宽度(dB)
     */
    void setThreshold(double thresholdDb, double ratio, double kneeDb = 0.0);

    /**
     * @brief 设置启动和释放时间
     * @param attackMs 启动时间(ms)
     * @param releaseMs 释放时间(ms)
     */
    void setAttackRelease(double attackMs, double releaseMs);

    /**
     * @brief 获取当前增益衰减量
     * @return 增益衰减(dB)
     */
    double gainReduction() const { return m_gainReductionDb; }

signals:
    /// 压缩处理完成信号
    void compressionCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_thresholdDb = -20.0;
    double m_ratio = 4.0;
    double m_kneeDb = 0.0;
    double m_attackMs = 10.0;
    double m_releaseMs = 100.0;
    double m_gainReductionDb = 0.0;
    double m_envelope = 0.0;
};
