/**
 * @file TTestCalculator.h
 * @brief T检验计算器 — 均值差异显著性检验
 *
 * 功能: 独立样本/配对样本/单样本t检验，输出t值/p值/自由度，
 *       统计计算次数/耗时。
 */
#ifndef TTESTCALCULATOR_H
#define TTESTCALCULATOR_H

#include <QObject>
#include <QVector>

class TTestCalculator : public QObject {
    Q_OBJECT
public:
    struct Result {
        double tValue = 0.0;
        double pValue = 1.0;
        double df = 0.0;
        double meanDiff = 0.0;
        bool significant = false;
    };

    struct Stats {
        quint64 totalTests = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit TTestCalculator(QObject* parent = nullptr);

    /** @brief 独立样本t检验 @param group1 样本1 @param group2 样本2 @return 结果 */
    Result independentTest(const QVector<double>& group1,
                           const QVector<double>& group2);

    /** @brief 配对样本t检验 @param before 前 @param after 后 @return 结果 */
    Result pairedTest(const QVector<double>& before,
                      const QVector<double>& after);

    /** @brief 单样本t检验 @param data 样本 @param mu 假设均值 @return 结果 */
    Result oneSampleTest(const QVector<double>& data, double mu = 0.0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testCompleted(double tValue, double pValue);

private:
    double computePValue(double t, double df);
    double incompleteBeta(double a, double b, double x);

    Stats m_stats;
    double m_timeSum;
};

#endif // TTESTCALCULATOR_H
