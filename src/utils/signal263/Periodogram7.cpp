/**
 * @file Periodogram7.cpp
 * @brief Periodogram7 实现
 *
 * 实现周期图：Lomb-Scargle非均匀采样时间序列与假警概率估计。
 */

#include "utils/signal263/Periodogram7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Periodogram7::Periodogram7(QObject *parent)
    : QObject(parent) {}
Periodogram7::~Periodogram7() = default;

/* ---- Configuration ---- */

void Periodogram7::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFreq = qMax(0.0, minFreq);
    m_maxFreq = qMax(m_minFreq + 1e-10, maxFreq);
}

void Periodogram7::setNumFreqBins(int bins) { m_numBins = qMax(2, bins); }

/* ---- Utility: linspace ---- */

QVector<double> Periodogram7::linspace(double start, double end, int n) const
{
    QVector<double> result(n);
    if (n == 1) { result[0] = start; return result; }
    double step = (end - start) / (n - 1);
    for (int i = 0; i < n; ++i)
        result[i] = start + i * step;
    return result;
}

/* ---- Log-gamma (Stirling's approximation) ---- */

double Periodogram7::logGamma(double x) const
{
    if (x <= 0) return 0.0;
    if (x < 0.5)
        return qLn(M_PI / qSin(M_PI * x)) - logGamma(1.0 - x);
    x -= 1.0;
    double ser = 1.0 + 76.18009172947146 / (x + 1.0)
                 - 86.50532032941677 / (x + 2.0)
                 + 24.01409824083091 / (x + 3.0)
                 - 1.231739572450155 / (x + 4.0)
                 + 0.001208650973866179 / (x + 5.0)
                 - 0.000005395239384953 / (x + 6.0);
    return 0.5 * qLn(2.0 * M_PI) + (x + 0.5) * qLn(x + 5.5) - (x + 5.5) + qLn(ser);
}

/* ---- Incomplete gamma function (series expansion) ---- */

double Periodogram7::incompleteGamma(double a, double x) const
{
    if (x < 0.0) return 0.0;
    if (x == 0.0) return 0.0;

    double sum = 1.0 / a;
    double term = 1.0 / a;
    for (int n = 1; n < 200; ++n) {
        term *= x / (a + n);
        sum += term;
        if (qAbs(term) < qAbs(sum) * 1e-12) break;
    }
    return sum * qExp(-x + a * qLn(x) - logGamma(a));
}

/* ---- Lomb-Scargle power at single frequency ---- */

double Periodogram7::lombScarglePower(double freq, const QVector<double>& t,
                                        const QVector<double>& y, double yMean) const
{
    int n = t.size();
    double omega = 2.0 * M_PI * freq;

    // Compute tau (weighted time reference)
    double sumSin2wt = 0.0, sumCos2wt = 0.0;
    for (int i = 0; i < n; ++i) {
        double w2t = 2.0 * omega * t[i];
        sumSin2wt += qSin(w2t);
        sumCos2wt += qCos(w2t);
    }
    double tau = qAtan2(sumSin2wt, sumCos2wt) / (2.0 * omega);

    // Compute Lomb-Scargle power
    double sumCosY = 0.0, sumSinY = 0.0;
    double sumCos2 = 0.0, sumSin2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = t[i] - tau;
        double c = qCos(omega * diff);
        double s = qSin(omega * diff);
        double dy = y[i] - yMean;
        sumCosY += c * dy;
        sumSinY += s * dy;
        sumCos2 += c * c;
        sumSin2 += s * s;
    }

    double power = 0.0;
    if (sumCos2 > 0.0) power += sumCosY * sumCosY / sumCos2;
    if (sumSin2 > 0.0) power += sumSinY * sumSinY / sumSin2;
    return power / 2.0;
}

/* ---- False alarm probability ---- */

double Periodogram7::falseAlarmProbability(double power, int n) const
{
    // FAP = (1 - (1 - e^{-z})^{N/2}) ≈ 1 - exp(-N_eff * e^{-z})
    // where z = power / variance
    double z = power;
    double neff = n / 2.0;
    // P_single = exp(-z)
    double pSingle = qExp(-z);
    // FAP for independent frequencies
    return 1.0 - qPow(1.0 - pSingle, neff);
}

/* ---- Estimate noise level ---- */

double Periodogram7::estimateNoiseLevel(const QVector<double>& power) const
{
    if (power.isEmpty()) return 1.0;
    QVector<double> sorted = power;
    std::sort(sorted.begin(), sorted.end());
    // Median as robust noise estimate
    int mid = sorted.size() / 2;
    return sorted[mid];
}

/* ---- FAP to Gaussian sigma ---- */

double Periodogram7::fapToSigma(double fap) const
{
    // Approximate inverse error function for Gaussian sigma
    if (fap <= 0.0) return 10.0;
    if (fap >= 1.0) return 0.0;
    // Use approximation: sigma ≈ sqrt(-2 * ln(fap))
    return qSqrt(-2.0 * qLn(fap));
}

/* ---- Compute periodogram ---- */

QVector<double> Periodogram7::compute(const QVector<double>& times,
                                        const QVector<double>& values) const
{
    QElapsedTimer timer;
    timer.start();

    int n = times.size();
    if (n != values.size() || n < 3) return {};

    // Compute mean of values
    double yMean = 0.0;
    for (double v : values) yMean += v;
    yMean /= n;

    // Generate frequency grid
    QVector<double> freqs = linspace(m_minFreq, m_maxFreq, m_numBins);
    QVector<double> power(m_numBins);

    double maxP = 0.0;
    double domFreq = 0.0;
    for (int i = 0; i < m_numBins; ++i) {
        power[i] = lombScarglePower(freqs[i], times, values, yMean);
        if (power[i] > maxP) {
            maxP = power[i];
            domFreq = freqs[i];
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numTimePoints = n;
    m_stats.numFreqBins = m_numBins;
    m_stats.maxPower = maxP;
    m_stats.dominantFrequency = domFreq;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit periodogramComputed(n, m_numBins, maxP, elapsed);
    return power;
}

/* ---- Detect peaks ---- */

QVector<Periodogram7::Peak> Periodogram7::detectPeaks(const QVector<double>& frequencies,
                                                         const QVector<double>& power,
                                                         double fapThreshold) const
{
    QVector<Peak> peaks;
    int n = power.size();
    if (n < 3) return peaks;

    double noise = estimateNoiseLevel(power);

    for (int i = 1; i < n - 1; ++i) {
        // Local maximum check
        if (power[i] > power[i - 1] && power[i] > power[i + 1]) {
            double normalizedPower = power[i] / qMax(noise, 1e-10);
            double fap = falseAlarmProbability(normalizedPower, m_stats.numTimePoints);
            if (fap <= fapThreshold) {
                Peak p;
                p.frequency = frequencies[i];
                p.power = power[i];
                p.falseAlarmProb = fap;
                p.significanceSigma = fapToSigma(fap);
                peaks.append(p);
                emit peakDetected(p.frequency, p.power, p.fap);
            }
        }
    }

    // Sort by power descending
    std::sort(peaks.begin(), peaks.end(),
              [](const Peak& a, const Peak& b) { return a.power > b.power; });
    return peaks;
}

/* ---- Frequency axis ---- */

QVector<double> Periodogram7::frequencyAxis() const
{
    return linspace(m_minFreq, m_maxFreq, m_numBins);
}

/* ---- Reset ---- */

void Periodogram7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
