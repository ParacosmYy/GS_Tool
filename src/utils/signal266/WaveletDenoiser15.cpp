/**
 * @file WaveletDenoiser15.cpp
 * @brief WaveletDenoiser15 实现
 *
 * 实现小波去噪：SURE自适应阈值选择与循环旋转平移不变去噪。
 */

#include "utils/signal266/WaveletDenoiser15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser15::WaveletDenoiser15(QObject *parent)
    : QObject(parent) {}

WaveletDenoiser15::~WaveletDenoiser15() = default;

/* ---- Configuration ---- */

void WaveletDenoiser15::setParameters(int decomposeLevels, WaveletType wavelet,
                                       int numCycleSpins, double maxThreshold)
{
    m_levels = qBound(1, decomposeLevels, 20);
    m_wavelet = wavelet;
    m_numSpins = qMax(0, numCycleSpins);
    m_maxThreshold = qMax(0.0, maxThreshold);
}

/* ---- Wavelet filter coefficients ---- */

QVector<double> WaveletDenoiser15::lowFilter() const
{
    // Daubechies low-pass decomposition filters
    switch (m_wavelet) {
    case WaveletType::Haar:
        return {1.0 / qSqrt(2.0), 1.0 / qSqrt(2.0)};
    case WaveletType::D4:
        return {0.4829629131445341, 0.8365163037378079,
                0.2241438680420134, -0.1294095225512603};
    case WaveletType::D6:
        return {0.3326705529500825, 0.8068915093110924, 0.4598775021184914,
                -0.1350110200102546, -0.0854412738820267, 0.0352262918857095};
    case WaveletType::D8:
        return {0.2303778133088964, 0.7148465705529154, 0.6308807679398587,
                -0.0279837694168599, -0.1870348117190931, 0.0308413818355607,
                0.0328830116668852, -0.0105974017850690};
    }
    return {1.0};
}

QVector<double> WaveletDenoiser15::highFilter() const
{
    auto lo = lowFilter();
    int n = lo.size();
    QVector<double> hi(n);
    for (int i = 0; i < n; ++i)
        hi[i] = ((i % 2 == 0) ? 1.0 : -1.0) * lo[n - 1 - i];
    return hi;
}

/* ---- Single-level forward DWT ---- */

void WaveletDenoiser15::forwardDWT(const QVector<double>& signal,
                                     QVector<double>& approx,
                                     QVector<double>& detail) const
{
    int n = signal.size();
    int fLen = lowFilter().size();
    auto lo = lowFilter();
    auto hi = highFilter();
    int outLen = (n + fLen - 1) / 2;

    approx.resize(outLen);
    detail.resize(outLen);
    for (int k = 0; k < outLen; ++k) {
        double a = 0.0, d = 0.0;
        for (int i = 0; i < fLen; ++i) {
            int idx = 2 * k + i;
            if (idx >= n) idx = 2 * n - idx - 2;  // Symmetric extension
            if (idx < 0) idx = -idx;
            a += lo[i] * signal[idx];
            d += hi[i] * signal[idx];
        }
        approx[k] = a;
        detail[k] = d;
    }
}

/* ---- Single-level inverse DWT ---- */

QVector<double> WaveletDenoiser15::inverseDWT(const QVector<double>& approx,
                                                const QVector<double>& detail) const
{
    int n = approx.size();
    int fLen = lowFilter().size();
    auto lo = lowFilter();
    auto hi = highFilter();
    int outLen = 2 * n;

    QVector<double> result(outLen, 0.0);
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < fLen; ++i) {
            int idx = k + i / 2;  // Upsample positions
            if (i % 2 == 0) {
                if (idx < outLen) result[idx] += lo[i] * approx[k] + hi[i] * detail[k];
            } else {
                if (idx < outLen) result[idx] += lo[i] * approx[k] + hi[i] * detail[k];
            }
        }
    }

    // Simpler upsampling convolution
    QVector<double> out(outLen, 0.0);
    for (int k = 0; k < n; ++k) {
        for (int j = 0; j < fLen; ++j) {
            int idx = 2 * k + 1 - j;
            if (idx >= 0 && idx < outLen) {
                out[idx] += lo[j] * approx[k] + hi[j] * detail[k];
            }
        }
    }
    return out;
}

/* ---- Full decomposition ---- */

void WaveletDenoiser15::decompose(const QVector<double>& signal)
{
    m_details.clear();
    QVector<double> current = signal;
    for (int lev = 0; lev < m_levels; ++lev) {
        QVector<double> approx, detail;
        forwardDWT(current, approx, detail);
        m_details.append(detail);
        current = approx;
    }
    m_approx = current;
}

/* ---- Full reconstruction ---- */

QVector<double> WaveletDenoiser15::reconstruct()
{
    QVector<double> current = m_approx;
    for (int lev = m_levels - 1; lev >= 0; --lev)
        current = inverseDWT(current, m_details[lev]);
    return current;
}

/* ---- SURE threshold (VisuShrink universal threshold) ---- */

double WaveletDenoiser15::computeSUREThreshold(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return 0.0;
    int n = coeffs.size();

    // Estimate noise level via MAD (median absolute deviation)
    QVector<double> absCoeffs(n);
    for (int i = 0; i < n; ++i) absCoeffs[i] = qAbs(coeffs[i]);
    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median = (n % 2 == 0)
        ? (absCoeffs[n / 2 - 1] + absCoeffs[n / 2]) / 2.0
        : absCoeffs[n / 2];
    double sigma = median / 0.6745;  // MAD to standard deviation

    // Universal threshold: sigma * sqrt(2 * ln(n))
    double t = sigma * qSqrt(2.0 * qLn(static_cast<double>(n)));
    if (m_maxThreshold > 0.0) t = qMin(t, m_maxThreshold);
    return t;
}

/* ---- Soft thresholding ---- */

QVector<double> WaveletDenoiser15::softThreshold(const QVector<double>& coeffs, double t)
{
    QVector<double> out(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) {
        double sign = (coeffs[i] >= 0) ? 1.0 : -1.0;
        out[i] = sign * qMax(0.0, qAbs(coeffs[i]) - t);
    }
    return out;
}

/* ---- Compute SNR ---- */

double WaveletDenoiser15::computeSNR(const QVector<double>& signal,
                                      const QVector<double>& noise)
{
    double sigPow = 0.0, noisePow = 0.0;
    int n = qMin(signal.size(), noise.size());
    for (int i = 0; i < n; ++i) {
        sigPow += signal[i] * signal[i];
        double diff = signal[i] - noise[i];
        noisePow += diff * diff;
    }
    if (noisePow < 1e-15) return 120.0;
    return 10.0 * qLog10(sigPow / noisePow);
}

/* ---- Denoise ---- */

QVector<double> WaveletDenoiser15::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    // Adjust levels to fit signal length
    int actualLevels = m_levels;
    int len = n;
    while (len >= 4 && actualLevels > 0) { len /= 2; actualLevels--; }
    actualLevels = qMin(m_levels, m_levels - actualLevels + 1);
    if (actualLevels < 1) actualLevels = 1;

    QVector<double> result;

    if (m_numSpins > 0) {
        // Cycle-spinning for translation invariance
        int spins = m_numSpins;
        QVector<double> accum(n, 0.0);
        double bestThreshold = 0.0;

        for (int s = 0; s < spins; ++s) {
            // Circular shift
            int shift = s;
            QVector<double> shifted(n);
            for (int i = 0; i < n; ++i)
                shifted[i] = signal[(i + shift) % n];

            // Decompose
            m_levels = actualLevels;
            decompose(shifted);

            // Compute threshold from finest detail level
            double t = computeSUREThreshold(m_details[0]);
            if (s == 0) bestThreshold = t;

            // Soft-threshold all detail levels
            for (int lev = 0; lev < m_details.size(); ++lev)
                m_details[lev] = softThreshold(m_details[lev], t);

            // Reconstruct
            QVector<double> rec = reconstruct();

            // Unshift
            for (int i = 0; i < n; ++i)
                accum[i] += rec[(i - shift + n) % n];
        }

        // Average over spins
        result.resize(n);
        for (int i = 0; i < n; ++i) result[i] = accum[i] / spins;
        m_threshold = bestThreshold;
    } else {
        // Standard single-pass denoising
        m_levels = actualLevels;
        decompose(signal);

        // SURE threshold from finest detail coefficients
        m_threshold = computeSUREThreshold(m_details[0]);

        // Soft-threshold all detail levels
        for (int lev = 0; lev < m_details.size(); ++lev)
            m_details[lev] = softThreshold(m_details[lev], m_threshold);

        result = reconstruct();
    }

    // Trim result to input length
    if (result.size() > n) result.resize(n);
    if (result.size() < n) result.resize(n, 0.0);

    double elapsed = timer.elapsed();
    double inSNR = computeSNR(signal, signal);
    double outSNR = computeSNR(signal, result);

    m_stats.signalLength = n;
    m_stats.numLevels = actualLevels;
    m_stats.threshold = m_threshold;
    m_stats.inputSNR = inSNR;
    m_stats.outputSNR = outSNR;
    m_stats.numSpins = m_numSpins;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit denoisingComplete(n, m_threshold, elapsed);
    return result;
}

/* ---- Accessors ---- */

QVector<QVector<double>> WaveletDenoiser15::detailCoefficients() const
{
    return m_details;
}

double WaveletDenoiser15::threshold() const { return m_threshold; }

/* ---- Reset ---- */

void WaveletDenoiser15::resetStatistics()
{
    m_details.clear();
    m_approx.clear();
    m_threshold = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
