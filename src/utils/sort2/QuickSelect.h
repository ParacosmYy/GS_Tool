/**
 * @file QuickSelect.h
 * @brief 快速选择算法 — 第K小元素/中位数
 *
 * 功能: 快速选择第k小/大元素，求中位数，带中位数的中位数回退策略，
 *       统计选择次数/比较次数/耗时。
 */
#ifndef QUICKSELECT_H
#define QUICKSELECT_H

#include <QObject>
#include <QVector>

class QuickSelect : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalSelects = 0;
        quint64 totalComparisons = 0;
        quint64 medianOfMediansUsed = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit QuickSelect(QObject* parent = nullptr);

    /** @brief 选择第k小元素 @param data 数据 @param k 索引(0-based) @return 第k小值 */
    double select(QVector<double>& data, int k);

    /** @brief 选择第k大元素 @param data 数据 @param k 索引(0-based) @return 第k大值 */
    double selectKthLargest(QVector<double>& data, int k);

    /** @brief 求中位数 @param data 数据(会被修改) @return 中位数 */
    double median(QVector<double>& data);

    /** @brief 求百分位数 @param data 数据 @param percentile 百分位(0~100) @return 值 */
    double percentile(QVector<double>& data, double percentile);

    /** @brief 多重选择(求多个分位数) @param data 数据 @param ks 索引列表 @return 值列表 */
    QVector<double> multiSelect(QVector<double>& data, const QVector<int>& ks);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void elementSelected(int k, double value);
    void medianComputed(double value);

private:
    double selectImpl(QVector<double>& data, int left, int right, int k);
    double medianOfMedians(QVector<double>& data, int left, int right);
    int partition(QVector<double>& data, int left, int right, double pivot);

    Stats m_stats;
    double m_timeSum;
};

#endif // QUICKSELECT_H
