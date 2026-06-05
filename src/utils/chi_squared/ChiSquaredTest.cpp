/**
 * @file ChiSquaredTest.cpp
 * @brief 卡方检验实现
 */

#include "ChiSquaredTest.h"
#include <QElapsedTimer>
#include <cmath>

ChiSquaredTest::ChiSquaredTest(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

double ChiSquaredTest::goodnessOfFit(const QVector<double>& observed,
                                       const QVector<double>& expected)
{
    QElapsedTimer timer;
    timer.start();

    int n = observed.size();
    if (n == 0) return 0.0;

    QVector<double> exp = expected;
    if (exp.isEmpty()) {
        double sum = 0.0;
        for (double o : observed) sum += o;
        double avg = sum / n;
        exp.fill(avg, n);
    }

    double chi2 = 0.0;
    for (int i = 0; i < n; ++i) {
        if (exp[i] > 0)
            chi2 += (observed[i] - exp[i]) * (observed[i] - exp[i]) / exp[i];
    }

    int df = n - 1;
    double p = pValue(chi2, df);

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(chi2, df, p);
    return chi2;
}

double ChiSquaredTest::independence(const QVector<QVector<double>>& table)
{
    QElapsedTimer timer;
    timer.start();

    int rows = table.size();
    if (rows == 0) return 0.0;
    int cols = table[0].size();

    QVector<double> rowSum(rows, 0.0), colSum(cols, 0.0);
    double total = 0.0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            rowSum[r] += table[r][c];
            colSum[c] += table[r][c];
            total += table[r][c];
        }
    }

    double chi2 = 0.0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double exp = rowSum[r] * colSum[c] / total;
            if (exp > 0)
                chi2 += (table[r][c] - exp) * (table[r][c] - exp) / exp;
        }
    }

    int df = (rows - 1) * (cols - 1);
    double p = pValue(chi2, df);

    m_stats.totalTests++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(chi2, df, p);
    return chi2;
}

double ChiSquaredTest::pValue(double chi2, int df)
{
    if (chi2 <= 0 || df <= 0) return 1.0;
    return 1.0 - cdf(chi2, df);
}

double ChiSquaredTest::cdf(double x, int df)
{
    if (x <= 0) return 0.0;
    double k = df / 2.0;
    return lowerIncompleteGamma(x / 2.0, k) / std::exp(gammaLn(k));
}

double ChiSquaredTest::yatesCorrection(double a, double b, double c, double d)
{
    double n = a + b + c + d;
    if (n == 0) return 0.0;
    double numer = std::abs(a * d - b * c) - n / 2.0;
    return numer * numer * n / ((a + b) * (c + d) * (a + c) * (b + d));
}

double ChiSquaredTest::gammaLn(double x)
{
    static double coeff[6] = {
        76.18009172947146, -86.50532032941677,
        24.01409824083091, -1.231739572450155,
        0.1208650973866179e-2, -0.5395239384953e-5
    };
    double y = x, tmp = x + 5.5;
    tmp -= (x + 0.5) * std::log(tmp);
    double ser = 1.000000000190015;
    for (int i = 0; i < 6; ++i)
        ser += coeff[i] / ++y;
    return -tmp + std::log(2.5066282746310005 * ser / x);
}

double ChiSquaredTest::lowerIncompleteGamma(double x, double a)
{
    if (x < a + 1) {
        double sum = 1.0 / a, term = 1.0 / a;
        for (int n = 1; n < 200; ++n) {
            term *= x / (a + n);
            sum += term;
            if (std::abs(term) < std::abs(sum) * 1e-12) break;
        }
        return sum * std::exp(-x + a * std::log(x) - gammaLn(a));
    }

    double b = x + 1.0 - a, c = 1e30, d = 1.0 / b, h = d;
    for (int i = 1; i < 200; ++i) {
        double an = -i * (i - a);
        b += 2.0;
        d = an * d + b;
        if (std::abs(d) < 1e-30) d = 1e-30;
        c = b + an / c;
        if (std::abs(c) < 1e-30) c = 1e-30;
        d = 1.0 / d;
        double del = d * c;
        h *= del;
        if (std::abs(del - 1.0) < 1e-12) break;
    }
    return std::exp(-x + a * std::log(x) - gammaLn(a)) * h;
}

ChiSquaredTest::Stats ChiSquaredTest::stats() const { return m_stats; }

void ChiSquaredTest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
