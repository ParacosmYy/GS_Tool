#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态范围扩展器实现 (版本5)
 *
 * 提供信号动态范围扩展功能，支持向下扩展、门限扩展和扩展比控制。
 */
class Expander5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalExpansionRuns = 0;     ///< 总扩展处理次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double maxExpansionDb = 0.0;    ///< 最大扩展量(dB)
    };

    explicit Expander5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频数据扩展
     * @param samples 输入采样数据
     * @return 扩展后的采样数据
     */
    QVector<double> process(const QVector<double>& samples);

    /**
     * @brief 设置扩展参数
     * @param thresholdDb 扩展阈值(dB)
     * @param ratio 扩展比
     * @param rangeDb 最大扩展范围(dB)
     */
    void setExpansion(double thresholdDb, double ratio, double rangeDb = 60.0);

    /**
     * @brief 设置启动和释放时间
     * @param attackMs 启动时间(ms)
     * @param releaseMs 释放时间(ms)
     * @param holdMs 保持时间(ms)
     */
    void setTiming(double attackMs, double releaseMs, double holdMs = 0.0);

    /**
     * @brief 获取当前增益量
     * @return 增益(dB)
     */
    double currentGainDb() const { return m_currentGainDb; }

signals:
    /// 扩展处理完成信号
    void expansionCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_thresholdDb = -40.0;
    double m_ratio = 2.0;
    double m_rangeDb = 60.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 100.0;
    double m_holdMs = 0.0;
    double m_currentGainDb = 0.0;
    double m_envelope = 0.0;
};
