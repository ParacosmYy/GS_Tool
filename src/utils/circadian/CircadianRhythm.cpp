/**
 * @file CircadianRhythm.cpp
 * @brief 昼夜节律分析器实现
 */

#include "CircadianRhythm.h"
#include <QElapsedTimer>
#include <cmath>

CircadianRhythm::CircadianRhythm(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

CircadianRhythm::FitResult CircadianRhythm::cosinor(
    const QVector<double>& timepoints,
    const QVector<double>& values,
    double period)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;
    result.period = period;
    int n = qMin(timepoints.size(), values.size());
    if (n < 3) {
        m_stats.totalFits++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;
        return result;
    }

    double omega = 2.0 * M_PI / period;
    double sumY = 0.0, sumCos = 0.0, sumSin = 0.0;
    double sumCos2 = 0.0, sumSin2 = 0.0, sumCosSin = 0.0;
    double sumYCos = 0.0, sumYSin = 0.0;

    for (int i = 0; i < n; ++i) {
        double y = values[i];
        double c = std::cos(omega * timepoints[i]);
        double s = std::sin(omega * timepoints[i]);
        sumY += y;
        sumCos += c;
        sumSin += s;
        sumCos2 += c * c;
        sumSin2 += s * s;
        sumCosSin += c * s;
        sumYCos += y * c;
        sumYSin += y * s;
    }

    /* 最小二乘: y = M + A*cos(wt) + B*sin(wt) */
    double denom = sumCos2 * sumSin2 - sumCosSin * sumCosSin;
    if (std::abs(denom) < 1e-15) {
        m_stats.totalFits++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;
        return result;
    }

    double M = sumY / n;
    double A = (sumYCos * sumSin2 - sumYSin * sumCosSin) / denom;
    double B = (sumYSin * sumCos2 - sumYCos * sumCosSin) / denom;

    result.mesor = M;
    result.amplitude = std::sqrt(A * A + B * B);
    result.acrophase = std::atan2(B, A);

    /* R²计算 */
    double ssTot = 0.0, ssRes = 0.0;
    double meanY = sumY / n;
    for (int i = 0; i < n; ++i) {
        double y = values[i];
        double fitted = M + A * std::cos(omega * timepoints[i])
                            + B * std::sin(omega * timepoints[i]);
        ssTot += (y - meanY) * (y - meanY);
        ssRes += (y - fitted) * (y - fitted);
    }
    result.rSquared = (ssTot > 0) ? 1.0 - ssRes / ssTot : 0.0;

    m_stats.totalFits++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(period, result.rSquared);
    return result;
}

CircadianRhythm::FitResult CircadianRhythm::multiCosinor(
    const QVector<double>& timepoints,
    const QVector<double>& values,
    const QVector<double>& periods)
{
    FitResult best;
    best.rSquared = -1.0;

    for (double p : periods) {
        FitResult candidate = cosinor(timepoints, values, p);
        if (candidate.rSquared > best.rSquared)
            best = candidate;
    }
    return best;
}

double CircadianRhythm::detectPeriod(const QVector<double>& timepoints,
                                       const QVector<double>& values,
                                       double minPeriod, double maxPeriod,
                                       int steps)
{
    QElapsedTimer timer;
    timer.start();

    double bestPeriod = minPeriod;
    double bestPower = -1e30;

    double step = (maxPeriod - minPeriod) / steps;
    for (int i = 0; i <= steps; ++i) {
        double p = minPeriod + i * step;
        double power = powerAtPeriod(timepoints, values, p);
        if (power > bestPower) {
            bestPower = power;
            bestPeriod = p;
        }
    }

    m_stats.totalPeriods++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalFits + m_stats.totalPeriods;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit periodDetected(bestPeriod);
    return bestPeriod;
}

double CircadianRhythm::phaseCoherence(const QVector<double>& phases)
{
    int n = phases.size();
    if (n == 0) return 0.0;

    double sumCos = 0.0, sumSin = 0.0;
    for (double p : phases) {
        sumCos += std::cos(p);
        sumSin += std::sin(p);
    }
    double R = std::sqrt(sumCos * sumCos + sumSin * sumSin) / n;
    return R;
}

double CircadianRhythm::interdailyStability(const QVector<double>& values)
{
    int n = values.size();
    if (n < 24) return 0.0;

    /* 按小时聚合 */
    int hours = 24;
    QVector<double> hourMean(hours, 0.0);
    QVector<int> hourCount(hours, 0);
    double grandMean = 0.0;

    for (int i = 0; i < n; ++i) {
        int h = i % hours;
        hourMean[h] += values[i];
        hourCount[h]++;
        grandMean += values[i];
    }
    grandMean /= n;

    for (int h = 0; h < hours; ++h)
        if (hourCount[h] > 0) hourMean[h] /= hourCount[h];

    /* 逐小时方差 */
    double varH = 0.0;
    for (int h = 0; h < hours; ++h)
        varH += (hourMean[h] - grandMean) * (hourMean[h] - grandMean);
    varH /= hours;

    /* 总方差 */
    double varT = 0.0;
    for (int i = 0; i < n; ++i)
        varT += (values[i] - grandMean) * (values[i] - grandMean);
    varT /= n;

    return (varT > 0) ? varH / varT : 0.0;
}

double CircadianRhythm::intradailyVariability(const QVector<double>& values)
{
    int n = values.size();
    if (n < 2) return 0.0;

    /* 一阶差分方差 */
    double diffVar = 0.0;
    for (int i = 1; i < n; ++i) {
        double d = values[i] - values[i - 1];
        diffVar += d * d;
    }
    diffVar /= (n - 1);

    /* 总方差 */
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += values[i];
    mean /= n;

    double var = 0.0;
    for (int i = 0; i < n; ++i)
        var += (values[i] - mean) * (values[i] - mean);
    var /= n;

    return (var > 0) ? diffVar / var : 0.0;
}

double CircadianRhythm::powerAtPeriod(const QVector<double>& tp,
                                        const QVector<double>& val,
                                        double period) const
{
    double omega = 2.0 * M_PI / period;
    int n = qMin(tp.size(), val.size());
    double sumCos = 0.0, sumSin = 0.0, sumCos2 = 0.0, sumSin2 = 0.0;
    double sumYCos = 0.0, sumYSin = 0.0;

    for (int i = 0; i < n; ++i) {
        double c = std::cos(omega * tp[i]);
        double s = std::sin(omega * tp[i]);
        sumCos += c; sumSin += s;
        sumCos2 += c * c; sumSin2 += s * s;
        sumYCos += val[i] * c; sumYSin += val[i] * s;
    }

    double A = (sumYCos - sumCos * sumYCos / n) / (sumCos2 - sumCos * sumCos / n + 1e-15);
    double B = (sumYSin - sumSin * sumYSin / n) / (sumSin2 - sumSin * sumSin / n + 1e-15);
    return A * A + B * B;
}

CircadianRhythm::Stats CircadianRhythm::stats() const { return m_stats; }

void CircadianRhythm::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
