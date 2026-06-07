/**
 * @file Periodogram3.cpp
 * @brief Periodogram3 实现
 *
 * 实现Lomb-Scargle周期图：非均匀采样功率谱、频率网格搜索、假警报概率。
 */

#include "utils/signal207/Periodogram3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Periodogram3::Periodogram3(QObject *parent) : QObject(parent) {}
Periodogram3::~Periodogram3() = default;

/* ---- Configuration ---- */

void Periodogram3::setMinFreq(double f) { m_minFreq = qMax(0.0, f); }
void Periodogram3::setMaxFreq(double f) { m_maxFreq = qMax(0.0, f); }
void Periodogram3::setNumFreqs(int n) { m_numFreqs = qMax(1, n); }
void Periodogram3::setOversampling(int factor) { m_oversampling = qMax(1, factor); }

/* ---- Build frequency grid ---- */

QVector<double> Periodogram3::buildFreqGrid(const QVector<double>& times) const
{
    int n = times.size();
    if (n < 2) return {};

    // Time span
    double tMin = *std::min_element(times.begin(), times.end());
    double tMax = *std::max_element(times.begin(), times.end());
    double T = tMax - tMin;
    if (T <= 0.0) return {};

    // Nyquist-like frequency for uneven sampling: average Nyquist
    double dtAvg = T / (n - 1);
    double fNyquist = 0.5 / dtAvg;

    double fMin = (m_minFreq > 0.0) ? m_minFreq : 1.0 / T;
    double fMax = (m_maxFreq > 0.0) ? m_maxFreq : fNyquist;

    // Oversampled grid
    int numPts = m_numFreqs * m_oversampling;
    QVector<double> freqs(numPts);
    double df = (fMax - fMin) / (numPts - 1);
    for (int i = 0; i < numPts; ++i)
        freqs[i] = fMin + i * df;

    return freqs;
}

/* ---- Lomb-Scargle power at single frequency ---- */

double Periodogram3::lombScarglePower(double freq, const QVector<double>& t,
                                        const QVector<double>& y, double yMean) const
{
    int n = t.size();
    double omega = 2.0 * M_PI * freq;

    // Compute tau: tan(2*omega*tau) = sum(sin(2*omega*t)) / sum(cos(2*omega*t))
    double sumSin2wt = 0.0, sumCos2wt = 0.0;
    for (int i = 0; i < n; ++i) {
        double angle = 2.0 * omega * t[i];
        sumSin2wt += qSin(angle);
        sumCos2wt += qCos(angle);
    }
    double tau = 0.5 * qAtan2(sumSin2wt, sumCos2wt) / omega;

    // Compute sums
    double sumCosY = 0.0, sumSinY = 0.0;
    double sumCos2 = 0.0, sumSin2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double yCent = y[i] - yMean;
        double arg = omega * (t[i] - tau);
        double c = qCos(arg);
        double s = qSin(arg);
        sumCosY += yCent * c;
        sumSinY += yCent * s;
        sumCos2 += c * c;
        sumSin2 += s * s;
    }

    // Lomb-Scargle normalized power
    double pX = (sumCos2 > 0.0) ? (sumCosY * sumCosY / sumCos2) : 0.0;
    double pY = (sumSin2 > 0.0) ? (sumSinY * sumSinY / sumSin2) : 0.0;

    // Variance of y
    double varY = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = y[i] - yMean;
        varY += d * d;
    }

    return (varY > 0.0) ? (pX + pY) / (2.0 * varY) : 0.0;
}

/* ---- Compute full periodogram ---- */

QVector<QPair<double, double>> Periodogram3::compute(const QVector<double>& times,
                                                       const QVector<double>& values) const
{
    QElapsedTimer timer;
    timer.start();
    int n = times.size();
    if (n < 2 || n != values.size()) return {};

    // Compute mean
    double yMean = 0.0;
    for (double v : values) yMean += v;
    yMean /= n;

    // Build frequency grid
    QVector<double> freqs = buildFreqGrid(times);
    QVector<QPair<double, double>> result;
    result.reserve(freqs.size());

    for (double f : freqs) {
        double power = lombScarglePower(f, times, values, yMean);
        result.append({f, power});
    }

    // Update stats (mutable via const_cast for stats tracking in const method)
    auto peak = findPeak(result);

    const_cast<Periodogram3*>(this)->m_stats.totalAnalyses++;
    const_cast<Periodogram3*>(this)->m_stats.numPoints = n;
    const_cast<Periodogram3*>(this)->m_stats.numFreqs = freqs.size();
    const_cast<Periodogram3*>(this)->m_stats.peakPower = peak.second;
    const_cast<Periodogram3*>(this)->m_timeSum += timer.elapsed();
    const_cast<Periodogram3*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalAnalyses;

    emit const_cast<Periodogram3*>(this)->analysisCompleted(peak.first, peak.second, timer.elapsed());
    return result;
}

/* ---- False alarm probability ---- */

double Periodogram3::falseAlarmProbability(double power, int n) const
{
    // For Lomb-Scargle: FAP ~ (1 - (1 - e^(-z))^M) where z = power * (n-1)/2
    // Simplified exponential approximation
    double z = power * (n - 1) / 2.0;
    int M = m_numFreqs * m_oversampling;
    double fapSingle = qExp(-z);
    double fap = 1.0 - qPow(1.0 - fapSingle, M);
    return qBound(0.0, fap, 1.0);
}

/* ---- Find peak ---- */

QPair<double, double> Periodogram3::findPeak(const QVector<QPair<double, double>>& periodogram) const
{
    if (periodogram.isEmpty()) return {0.0, 0.0};
    double bestFreq = 0.0, bestPower = -1.0;
    for (const auto& p : periodogram) {
        if (p.second > bestPower) { bestPower = p.second; bestFreq = p.first; }
    }
    return {bestFreq, bestPower};
}

/* ---- Reset ---- */

void Periodogram3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
