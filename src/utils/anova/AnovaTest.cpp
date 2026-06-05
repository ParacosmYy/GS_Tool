/**
 * @file AnovaTest.cpp
 * @brief 方差分析(ANOVA)实现
 */

#include "utils/anova/AnovaTest.h"

#include <QtMath>
#include <QElapsedTimer>
#include <cmath>

AnovaTest::AnovaTest(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

AnovaTest::Result AnovaTest::oneWay(const QVector<QVector<double>>& groups)
{
    Result r;
    int k = groups.size();
    if (k < 2) return r;

    QElapsedTimer timer;
    timer.start();

    double grandMean = 0.0;
    int totalN = 0;
    QVector<double> groupMeans(k);

    for (int g = 0; g < k; ++g) {
        double sum = 0.0;
        for (double v : groups[g]) sum += v;
        groupMeans[g] = (groups[g].size() > 0) ? sum / groups[g].size() : 0.0;
        grandMean += sum;
        totalN += groups[g].size();
    }
    if (totalN == 0) return r;
    grandMean /= totalN;

    double ssBetween = 0.0, ssWithin = 0.0;
    for (int g = 0; g < k; ++g) {
        int ni = groups[g].size();
        if (ni == 0) continue;
        ssBetween += ni * (groupMeans[g] - grandMean) * (groupMeans[g] - grandMean);
        for (double v : groups[g]) {
            double diff = v - groupMeans[g];
            ssWithin += diff * diff;
        }
    }

    double dfBetween = k - 1;
    double dfWithin = totalN - k;

    r.ssBetween = ssBetween;
    r.ssWithin = ssWithin;
    r.dfBetween = dfBetween;
    r.dfWithin = dfWithin;

    double msBetween = (dfBetween > 0) ? ssBetween / dfBetween : 0.0;
    double msWithin = (dfWithin > 0) ? ssWithin / dfWithin : 1e-15;

    r.fStatistic = (msWithin > 0) ? msBetween / msWithin : 0.0;
    r.pValue = fDistPValue(r.fStatistic, dfBetween, dfWithin);
    r.significant = r.pValue < 0.05;

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(r.fStatistic, r.pValue);
    return r;
}

double AnovaTest::fDistPValue(double f, double df1, double df2)
{
    if (f <= 0 || df1 <= 0 || df2 <= 0) return 1.0;
    double x = df2 / (df2 + df1 * f);
    double a = df2 / 2.0, b = df1 / 2.0;

    double logBeta = std::lgamma(a) + std::lgamma(b) - std::lgamma(a + b);
    double prefix = std::exp(a * std::log(x) + b * std::log(1.0 - x) - logBeta) / a;

    double sum = 1.0, term = 1.0;
    for (int i = 1; i <= 100; ++i) {
        term *= (a + i - 1.0) * (1.0 - x) / (a + 2.0 * i - 1.0 + b - 1.0);
        sum += term;
        if (qAbs(term) < 1e-15) break;
    }
    return qBound(0.0, prefix * sum, 1.0);
}

void AnovaTest::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
