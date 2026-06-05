/**
 * @file MedianAbsoluteDeviation.h
 * @brief 中位数绝对偏差(MAD)异常值检测器
 *
 * 功能: 基于MAD的鲁棒异常值检测，使用中位数和MAD计算鲁棒Z分数，
 *       识别偏离中位数过远的数据点。对异常值比均值/标准差更鲁棒。
 *
 * 协作: OutlierDetector(多方法异常检测) / DataQualityMonitor(数据质量)
 */
#ifndef MEDIANABSOLUTEDEVIATION_H
#define MEDIANABSOLUTEDEVIATION_H

#include <QObject>
#include <QVector>

/**
 * @brief 中位数绝对偏差异常值检测器
 *
 * MAD = median(|xi - median(x)|)
 * 鲁棒Z分数 = 0.6745 * (xi - median) / MAD
 * 阈值默认3.5，对应约99.7%置信区间
 */
class MedianAbsoluteDeviation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputed = 0;         ///< 累计MAD计算次数
        quint64 totalOutliers = 0;         ///< 累计检测到的异常值数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit MedianAbsoluteDeviation(QObject* parent = nullptr);

    /**
     * @brief 计算数据集的MAD值
     * @param data 输入数据
     * @return MAD值(中位数绝对偏差)
     */
    double compute(const QVector<double>& data);

    /**
     * @brief 检测异常值索引
     * @param data 输入数据
     * @param threshold 鲁棒Z分数阈值(默认3.5)
     * @return 异常值在data中的索引列表
     */
    QVector<int> detectOutliers(const QVector<double>& data,
                                double threshold = 3.5);

    /**
     * @brief 计算单个值的鲁棒Z分数
     * @param value 待计算的值
     * @param median 数据集中位数
     * @param mad 中位数绝对偏差
     * @return 鲁棒Z分数
     */
    double robustZScore(double value, double median, double mad) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 异常值检测完成信号 @param count 检测到的异常值数量 */
    void outliersDetected(int count);

private:
    /**
     * @brief 计算中位数(排序后取中间值)
     * @param data 输入数据
     * @return 中位数
     */
    static double computeMedian(const QVector<double>& data);

    Stats m_stats;              ///< 统计信息
    double m_timeSumMs = 0.0;   ///< 累计耗时(ms)
};

#endif // MEDIANABSOLUTEDEVIATION_H
