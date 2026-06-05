/**
 * @file ChiSquareTest.h
 * @brief 卡方检验 — 独立性/拟合优度检验
 */
#ifndef CHISQUARETEST_H
#define CHISQUARETEST_H

#include <QObject>
#include <QVector>
#include <QVector>

class ChiSquareTest : public QObject {
    Q_OBJECT
public:
    struct Result {
        double chiSquare = 0.0;
        double pValue = 1.0;
        double df = 0.0;
        bool significant = false;
    };

    struct Stats {
        quint64 totalTests = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit ChiSquareTest(QObject* parent = nullptr);

    /** @brief 拟合优度检验 @param observed 观测频率 @param expected 期望频率 @return 结果 */
    Result goodnessOfFit(const QVector<double>& observed,
                         const QVector<double>& expected);

    /** @brief 独立性检验 @param contingencyTable 列联表[行][列] @return 结果 */
    Result independenceTest(const QVector<QVector<double>>& contingencyTable);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void testCompleted(double chiSquare, double pValue);

private:
    double chiSquarePValue(double x, double df);

    Stats m_stats;
    double m_timeSum;
};

#endif // CHISQUARETEST_H
