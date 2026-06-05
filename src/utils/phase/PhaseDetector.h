/**
 * @file PhaseDetector.h
 * @brief 相位检测器 — 希尔伯特变换相位提取
 *
 * 功能: 提取信号的瞬时相位、瞬时频率和相位差。
 *       支持相位解卷绕(unwrapping)和相位同步检测。
 *
 * 协作: HilbertTransform(解析信号) / CrossCorrelator(延迟估计)
 */
#ifndef PHASEDETECTOR_H
#define PHASEDETECTOR_H

#include <QObject>
#include <QVector>

/**
 * @brief 相位检测器
 */
class PhaseDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetections = 0;    ///< 累计检测次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit PhaseDetector(QObject* parent = nullptr);

    /** @brief 从解析信号提取瞬时相位
     *  @param analytic 解析信号(复数实部虚部交替存储)
     *  @return 瞬时相位(弧度) */
    QVector<double> extractPhase(const QVector<double>& analytic) const;

    /** @brief 相位解卷绕
     *  @param phase 卷绕相位
     *  @return 解卷绕相位 */
    QVector<double> unwrapPhase(const QVector<double>& phase) const;

    /** @brief 计算瞬时频率
     *  @param unwrappedPhase 解卷绕相位
     *  @param sampleRate 采样率
     *  @return 瞬时频率(Hz) */
    QVector<double> instantaneousFrequency(
        const QVector<double>& unwrappedPhase,
        double sampleRate) const;

    /** @brief 计算两个信号的相位差
     *  @param phase1 信号1的解卷绕相位
     *  @param phase2 信号2的解卷绕相位
     *  @return 相位差(弧度) */
    QVector<double> phaseDifference(const QVector<double>& phase1,
                                    const QVector<double>& phase2) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检测完成 @param sampleCount 样本数 */
    void detectionCompleted(int sampleCount);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // PHASEDETECTOR_H
