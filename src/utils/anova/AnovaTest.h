/**
 * @file AnovaTest.h
 * @brief 方差分析(ANOVA) — 多组均值差异检验
 */
#ifndef ANOVATEST_H
#define ANOVATEST_H

#include <QObject>
#include <QVector>

class AnovaTest : public QObject {
    Q_OBJECT
public:
    struct Result {
        double fStatistic = 0.0;
        double pValue = 1.0;
        double dfBetween = 0.0;
        double dfWithin = 0.0;
        double ssBetween = 0.0;
        double ssWithin = 0.0;
        bool significant = false;
    };

    struct Stats {
        quint64 totalTests = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit AnovaTest(QObject* parent = nullptr);

    /** @brief 单因素ANOVA @param groups 各组数据 @return 结果 */
    Result oneWay(const QVector<QVector<double>>& groups);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testCompleted(double fStat, double pValue);

private:
    double fDistPValue(double f, double df1, double df2);

    Stats m_stats;
    double m_timeSum;
};

#endif // ANOVATEST_H
