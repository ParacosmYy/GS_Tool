/**
 * @file PeakDetector2.h
 * @brief 峰值检测器V2 — 阈值+间距+prominences
 *
 * 功能: 多条件峰值检测，支持最小阈值、最小间距、峰值prominences计算。
 *       prominences用于衡量峰值相对于周围信号的突出程度，
 *       可用于筛选真实峰值、去除虚假峰值。
 *
 * 协作: PeakDetector(基础检测) / SpectrumAnalyzer(频谱峰值)
 */
#ifndef PEAKDETECTOR2_H
#define PEAKDETECTOR2_H

#include <QObject>
#include <QVector>

/**
 * @brief 峰值检测器V2 — 阈值+间距+prominences
 */
class PeakDetector2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetections = 0;       ///< 累计检测次数
        quint64 totalPeaksFound = 0;       ///< 累计发现峰值数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit PeakDetector2(QObject* parent = nullptr);

    /** @brief 检测信号峰值
     *  @param signal 输入信号
     *  @param threshold 最小峰值阈值
     *  @param minDistance 最小峰值间距(样本数)
     *  @return 峰值索引列表 */
    QVector<int> detect(const QVector<double>& signal,
                        double threshold = 0.0,
                        int minDistance = 1);

    /** @brief 计算峰值prominences
     *  @param signal 输入信号
     *  @param peaks 峰值索引列表
     *  @return prominences列表(与peaks等长) */
    QVector<double> prominences(const QVector<double>& signal,
                                const QVector<int>& peaks);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检测完成 @param numPeaks 发现峰值数 */
    void detectionCompleted(int numPeaks);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // PEAKDETECTOR2_H
