/**
 * @file PeakHoldDetector.h
 * @brief 峰值保持检测器 — 可配置间隔的最大/最小值跟踪
 *
 * 功能: 在可配置的采样间隔内跟踪信号的最大值和最小值，
 *       当峰值/谷值变化时发出信号，适用于实时信号监控、
 *       包络检测和极值统计。
 *
 * 协作: PeakDetector(峰值检测) / EnvelopeDetector(包络检测)
 */
#ifndef PEAKHOLDDETECTOR_H
#define PEAKHOLDDETECTOR_H

#include <QObject>

/**
 * @brief 峰值保持检测器 — 间隔极值跟踪
 */
class PeakHoldDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalUpdates = 0;         ///< 累计更新次数
        quint64 totalPeakChanges = 0;     ///< 累计峰值变化次数
    };

    explicit PeakHoldDetector(QObject* parent = nullptr);

    /** @brief 更新采样值
     *  @param value 采样值 */
    void update(double value);

    /** @brief 获取当前峰值(最大值) @return 当前峰值 */
    double currentPeak() const;

    /** @brief 获取当前谷值(最小值) @return 当前谷值 */
    double currentTrough() const;

    /** @brief 设置保持时间(采样数)
     *  @param samples 保持样本数(0=无限保持) */
    void setHoldTime(int samples);

    /** @brief 重置检测器状态 */
    void reset();

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 峰值变化 @param newPeak 新峰值 */
    void peakChanged(double newPeak);

    /** @brief 谷值变化 @param newTrough 新谷值 */
    void troughChanged(double newTrough);

private:
    double m_peak;                 ///< 当前峰值
    double m_trough;               ///< 当前谷值
    int m_holdTime;                ///< 保持时间(采样数)
    int m_peakAge;                 ///< 峰值年龄(采样计数)
    int m_troughAge;               ///< 谷值年龄(采样计数)
    bool m_initialized;            ///< 是否已初始化

    Stats m_stats;                 ///< 统计
};

#endif // PEAKHOLDDETECTOR_H
