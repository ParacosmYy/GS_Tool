/**
 * @file WaveletDenoiser4.cpp
 * @brief WaveletDenoiser4 实现
 *
 * 实现平稳小波去噪：SWT非抽取变换、SURE阈值估计、软硬阈值、Haar/D4基。
 */

#include "utils/signal190/WaveletDenoiser4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser4::WaveletDenoiser4(QObject *parent) : QObject(parent) {}
WaveletDenoiser4::~WaveletDenoiser4() = default;

/* ---- Configuration ---- */

void WaveletDenoiser4::setDecompositionLevels(int levels)
{
    m_levels = qMax(1, levels);
}
void WaveletDenoiser4::setThresholdType(ThresholdType type) { m_thresholdType = type; }
void WaveletDenoiser4::setWaveletBase(WaveletBase base) { m_waveletBase = base; }

/* ---- Circular shift for translation invariance ---- */

QVector<double> WaveletDenoiser4::circShift(const QVector<double>& in,
                                             int shift) const
{
    int n = in.size();
    if (n == 0) return {};
    shift = ((shift % n) + n) % n;
    QVector<double> out(n);
    for (int i = 0; i < n; ++i)
        out[i] = in[(i + shift) % n];
    return out;
}

/* ---- Haar low/high pass decomposition ---- */

void WaveletDenoiser4::haarFilter(const QVector<double>& in,
                                   QVector<double>& approx,
                                   QVector<double>& detail) const
{
    int n = in.size();
    int half = n / 2;
    approx.resize(half);
    detail.resize(half);
    for (int i = 0; i < half; ++i) {
        approx[i] = (in[2 * i] + in[2 * i + 1]) / qSqrt(2.0);
        detail[i] = (in[2 * i] - in[2 * i + 1]) / qSqrt(2.0);
    }
}

/* ---- D4 low/high pass decomposition ---- */

void WaveletDenoiser4::d4Filter(const QVector<double>& in,
                                 QVector<double>& approx,
                                 QVector<double>& detail) const
{
    // Daubechies 4-tap coefficients
    static const double h0 = 0.6830127, h1 =  1.1830127;
    static const double h2 = 0.3169873, h3 = -0.1830127;
    static const double g0 = h3, g1 = -h2, g2 = h1, g3 = -h0;
    static const double invSqrt2 = 1.0 / qSqrt(2.0);

    int n = in.size();
    int half = n / 2;
    approx.resize(half);
    detail.resize(half);
    for (int i = 0; i < half; ++i) {
        int i0 = (2 * i) % n;
        int i1 = (2 * i + 1) % n;
        int i2 = (2 * i + 2) % n;
        int i3 = (2 * i + 3) % n;
        approx[i] = (h0 * in[i0] + h1 * in[i1] + h2 * in[i2] + h3 * in[i3])
                    * invSqrt2;
        detail[i] = (g0 * in[i0] + g1 * in[i1] + g2 * in[i2] + g3 * in[i3])
                    * invSqrt2;
    }
}

/* ---- Haar reconstruction ---- */

QVector<double> WaveletDenoiser4::haarReconstruct(
    const QVector<double>& approx, const QVector<double>& detail) const
{
    int half = approx.size();
    int n = 2 * half;
    QVector<double> out(n);
    for (int i = 0; i < half; ++i) {
        out[2 * i] = (approx[i] + detail[i]) / qSqrt(2.0);
        out[2 * i + 1] = (approx[i] - detail[i]) / qSqrt(2.0);
    }
    return out;
}

/* ---- D4 reconstruction ---- */

QVector<double> WaveletDenoiser4::d4Reconstruct(
    const QVector<double>& approx, const QVector<double>& detail) const
{
    static const double rh0 = 1.1830127, rh1 = 0.6830127;
    static const double rh2 = -0.1830127, rh3 = 0.3169873;
    static const double invSqrt2 = 1.0 / qSqrt(2.0);

    int half = approx.size();
    int n = 2 * half;
    QVector<double> out(n, 0.0);
    for (int i = 0; i < half; ++i) {
        double a = approx[i] * invSqrt2;
        double d = detail[i] * invSqrt2;
        int i0 = (2 * i) % n;
        int i1 = (2 * i + 1) % n;
        int i2 = (2 * i + 2) % n;
        int i3 = (2 * i + 3) % n;
        out[i0] += rh3 * a + rh2 * d;
        out[i1] += rh2 * a + rh0 * d;
        out[i2] += rh0 * a + rh1 * d;
        out[i3] += rh1 * a + rh3 * d;
    }
    return out;
}

/* ---- SWT forward transform ---- */

QVector<QVector<double>> WaveletDenoiser4::swtForward(
    const QVector<double>& signal) const
{
    int n = signal.size();
    QVector<QVector<double>> coeffs;
    coeffs.reserve(m_levels + 1);

    QVector<double> current = signal;
    for (int lev = 0; lev < m_levels; ++lev) {
        QVector<double> approx, detail;
        if (m_waveletBase == Haar)
            haarFilter(current, approx, detail);
        else
            d4Filter(current, approx, detail);
        coeffs.append(detail); // Detail coefficients at this level
        current = approx;
    }
    coeffs.append(current); // Final approximation
    return coeffs;
}

/* ---- SWT inverse ---- */

QVector<double> WaveletDenoiser4::swtInverse(
    const QVector<QVector<double>>& coeffs) const
{
    if (coeffs.isEmpty()) return {};

    int levels = coeffs.size() - 1;
    QVector<double> approx = coeffs[levels]; // Final approximation

    for (int lev = levels - 1; lev >= 0; --lev) {
        const auto& detail = coeffs[lev];
        if (m_waveletBase == Haar)
            approx = haarReconstruct(approx, detail);
        else
            approx = d4Reconstruct(approx, detail);
    }
    return approx;
}

/* ---- SURE threshold estimation ---- */

double WaveletDenoiser4::estimateSureThreshold(
    const QVector<double>& detailCoeffs) const
{
    int n = detailCoeffs.size();
    if (n == 0) return 0.0;

    // Sort absolute values
    QVector<double> absCoeffs(n);
    for (int i = 0; i < n; ++i) absCoeffs[i] = qFabs(detailCoeffs[i]);
    std::sort(absCoeffs.begin(), absCoeffs.end());

    // SURE: S(t) = n - 2*count(|x|<=t) + sum(min(|x|,t)^2)
    double sigma2 = 0.0;
    for (int i = 0; i < n; ++i) sigma2 += detailCoeffs[i] * detailCoeffs[i];
    sigma2 /= n; // Variance estimate

    double bestRisk = 1e30;
    double bestT = 0.0;

    for (int i = 0; i < n; ++i) {
        double t = absCoeffs[i];
        double risk = n * sigma2;
        int countBelow = 0;
        for (int j = 0; j < n; ++j) {
            double c = qFabs(detailCoeffs[j]);
            if (c <= t) countBelow++;
            risk += qMin(c, t) * qMin(c, t) - sigma2;
        }
        risk -= 2.0 * sigma2 * countBelow;

        if (risk < bestRisk) { bestRisk = risk; bestT = t; }
    }
    return bestT;
}

/* ---- Apply threshold ---- */

QVector<double> WaveletDenoiser4::applyThreshold(const QVector<double>& coeffs,
                                                  double threshold) const
{
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) {
        double c = coeffs[i];
        double ac = qFabs(c);
        if (ac <= threshold) {
            result[i] = 0.0;
        } else if (m_thresholdType == SoftThreshold) {
            result[i] = (c > 0 ? 1 : -1) * (ac - threshold);
        } else {
            result[i] = c; // Hard threshold: keep as is
        }
    }
    return result;
}

/* ---- Main denoise ---- */

QVector<double> WaveletDenoiser4::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    auto coeffs = swtForward(signal);

    // Estimate and apply threshold at each level
    double globalThreshold = 0.0;
    for (int lev = 0; lev < m_levels; ++lev) {
        double t = estimateSureThreshold(coeffs[lev]);
        coeffs[lev] = applyThreshold(coeffs[lev], t);
        globalThreshold = qMax(globalThreshold, t);
    }

    QVector<double> result = swtInverse(coeffs);

    // Trim/pad to match input length
    result.resize(signal.size());

    m_stats.totalDenoiseOps++;
    m_stats.signalLength = signal.size();
    m_stats.decompositionLevels = m_levels;
    m_stats.threshold = globalThreshold;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoiseOps;

    emit denoiseCompleted(m_levels, globalThreshold, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
