/**
 * @file KsTestCalculator.h
 * @brief Kolmogorov-Smirnov检验 — 分布一致性检验
 *
 * 功能: 单样本/双样本KS检验，计算D统计量和p值，
 *       统计检验次数/耗时。
 */
#ifndef KSTESTCALCULATOR_H
#define KSTESTCALCULATOR_H

#include <QObject>
#include <QVector>

class KsTestCalculator : public QObject {
    Q_OBJECT
public:
    struct Result {
        double dStatistic = 0.0;
        double pValue = 1.0;
        bool significant = false;
    };

    struct Stats {
        quint64 totalTests = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit KsTestCalculator(QObject* parent = nullptr);

    /** @brief 双样本KS检验 @param sample1 样本1 @param sample2 样本2 @return 结果 */
    Result twoSampleTest(const QVector<double>& sample1,
                         const QVector<double>& sample2);

    /** @brief 单样本KS检验(均匀分布) @param data 样本 @return 结果 */
    Result oneSampleUniform(const QVector<double>& data);

    /** @brief 单样本KS检验(正态分布) @param data 样本 @return 结果 */
    Result oneSampleNormal(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testCompleted(double dStat, double pValue);

private:
    double computePValue(double d, int n1, int n2);
    double kolmogorovCdf(double x);

    Stats m_stats;
    double m_timeSum;
};

#endif // KSTESTCALCULATOR_H
