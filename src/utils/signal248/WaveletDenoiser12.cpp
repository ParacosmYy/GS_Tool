/**
 * @file WaveletDenoiser12.cpp
 * @brief WaveletDenoiser12 实现
 *
 * 实现小波去噪：SURE阈值选择与循环旋转的平移不变去噪。
 */

#include "utils/signal248/WaveletDenoiser12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser12::WaveletDenoiser12(QObject *parent) : QObject(parent) {}
WaveletDenoiser12::~WaveletDenoiser12() = default;

/* ---- Configuration ---- */

void WaveletDenoiser12::setWavelet(Wavelet w) { m_wavelet = w; }
void WaveletDenoiser12::setLevels(int levels) { m_levels = qMax(1, levels); }
void WaveletDenoiser12::setCycleSpins(int spins) { m_spins = qMax(1, spins); }

/* ---- Wavelet filter coefficients ---- */

QVector<double> WaveletDenoiser12::lowDecompFilter() const
{
    // Haar low-pass decomposition filter
    if (m_wavelet == Haar) return {1.0 / qSqrt(2.0), 1.0 / qSqrt(2.0)};
    if (m_wavelet == DB2) return {0.4829629, 0.8365163, 0.2241439, -0.1294095};
    // DB4 (8 coefficients)
    return {0.2303778, 0.7148466, 0.6308808, -0.0279838,
            -0.1870348, 0.0308414, 0.0328830, -0.0105974};
}

QVector<double> WaveletDenoiser12::highDecompFilter() const
{
    QVector<double> h = lowDecompFilter();
    int n = h.size();
    QVector<double> g(n);
    for (int i = 0; i < n; ++i)
        g[i] = (i % 2 == 0 ? 1.0 : -1.0) * h[n - 1 - i];
    return g;
}

QVector<double> WaveletDenoiser12::lowReconFilter() const { return lowDecompFilter(); }
QVector<double> WaveletDenoiser12::highReconFilter() const { return highDecompFilter(); }

/* ---- Circular convolution ---- */

QVector<double> WaveletDenoiser12::circConv(const QVector<double>& x,
                                             const QVector<double>& h)
{
    int n = x.size();
    int m = h.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int k = 0; k < m; ++k)
            y[i] += h[k] * x[(i - k + n) % n];
    return y;
}

/* ---- Downsample ---- */

QVector<double> WaveletDenoiser12::downsample(const QVector<double>& x)
{
    int n = (x.size() + 1) / 2;
    QVector<double> y(n);
    for (int i = 0; i < n; ++i) y[i] = x[2 * i];
    return y;
}

/* ---- Upsample ---- */

QVector<double> WaveletDenoiser12::upsample(const QVector<double>& x, int targetLen)
{
    QVector<double> y(targetLen, 0.0);
    for (int i = 0; i < x.size() && 2 * i < targetLen; ++i)
        y[2 * i] = x[i];
    return y;
}

/* ---- Decompose ---- */

QVector<QVector<double>> WaveletDenoiser12::decompose(const QVector<double>& signal) const
{
    QVector<QVector<double>> coeffs;
    QVector<double> approx = signal;
    auto h = lowDecompFilter();
    auto g = highDecompFilter();

    for (int lev = 0; lev < m_levels; ++lev) {
        QVector<double> filteredH = circConv(approx, h);
        QVector<double> filteredG = circConv(approx, g);
        QVector<double> detail = downsample(filteredG);
        approx = downsample(filteredH);
        coeffs.append(detail);
    }
    coeffs.append(approx);  // Final approximation
    return coeffs;
}

/* ---- Reconstruct ---- */

QVector<double> WaveletDenoiser12::reconstruct(const QVector<QVector<double>>& coeffs) const
{
    if (coeffs.size() < 2) return QVector<double>();

    auto h = lowReconFilter();
    auto g = highReconFilter();

    // Start from coarsest approximation
    QVector<double> approx = coeffs.last();
    int targetLen = approx.size() * 2;

    for (int lev = coeffs.size() - 2; lev >= 0; --lev) {
        targetLen = approx.size() * 2;
        QVector<double> upA = upsample(approx, targetLen);
        QVector<double> upD = upsample(coeffs[lev], targetLen);

        QVector<double> convA = circConv(upA, h);
        QVector<double> convD = circConv(upD, g);

        approx.resize(targetLen);
        for (int i = 0; i < targetLen; ++i)
            approx[i] = convA[i] + convD[i];
    }
    return approx;
}

/* ---- SURE threshold ---- */

double WaveletDenoiser12::sureThreshold(const QVector<double>& coeffs) const
{
    int n = coeffs.size();
    if (n == 0) return 0.0;

    // Sort absolute values
    QVector<double> absC(n);
    for (int i = 0; i < n; ++i) absC[i] = qAbs(coeffs[i]);
    std::sort(absC.begin(), absC.end());

    // SURE: minimize risk(t) = n - 2*|{i:|c_i|<=t}| + sum min(|c_i|,t)^2
    double bestRisk = 1e18;
    double bestT = 0.0;

    for (int k = 0; k < n; ++k) {
        double t = absC[k];
        double risk = n;
        int below = 0;
        for (int i = 0; i < n; ++i) {
            if (absC[i] <= t) below++;
            else risk += (absC[i] - t) * (absC[i] - t);
        }
        risk -= 2 * below;
        if (risk < bestRisk) { bestRisk = risk; bestT = t; }
    }
    return bestT;
}

/* ---- Soft thresholding ---- */

QVector<double> WaveletDenoiser12::softThreshold(const QVector<double>& x, double t)
{
    QVector<double> y(x.size());
    for (int i = 0; i < x.size(); ++i)
        y[i] = (x[i] > t ? x[i] - t : (x[i] < -t ? x[i] + t : 0.0));
    return y;
}

/* ---- Circular shift ---- */

QVector<double> WaveletDenoiser12::circShift(const QVector<double>& x, int shift)
{
    int n = x.size();
    shift = ((shift % n) + n) % n;
    QVector<double> y(n);
    for (int i = 0; i < n; ++i)
        y[i] = x[(i + shift) % n];
    return y;
}

/* ---- Estimate SNR ---- */

double WaveletDenoiser12::estimateSNR(const QVector<double>& clean,
                                       const QVector<double>& noisy)
{
    double signalPower = 0.0, noisePower = 0.0;
    int n = qMin(clean.size(), noisy.size());
    for (int i = 0; i < n; ++i) {
        signalPower += clean[i] * clean[i];
        double noise = noisy[i] - clean[i];
        noisePower += noise * noise;
    }
    if (noisePower < 1e-30) return 100.0;
    return 10.0 * qLn(signalPower / noisePower) / qLn(10.0);
}

/* ---- Denoise with cycle-spinning ---- */

QVector<double> WaveletDenoiser12::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) return signal;

    // Translation-invariant denoising via cycle-spinning
    QVector<double> accumulator(n, 0.0);
    double bestThreshold = 0.0;

    for (int s = 0; s < m_spins; ++s) {
        int shift = static_cast<int>(n * s / m_spins);
        QVector<double> shifted = circShift(signal, shift);

        // Decompose
        auto coeffs = decompose(shifted);

        // Apply SURE threshold to detail coefficients (not final approx)
        for (int lev = 0; lev < coeffs.size() - 1; ++lev) {
            double t = sureThreshold(coeffs[lev]);
            coeffs[lev] = softThreshold(coeffs[lev], t);
            bestThreshold += t;
        }

        // Reconstruct
        QVector<double> recon = reconstruct(coeffs);

        // Unshift and accumulate
        recon = circShift(recon, -shift);
        for (int i = 0; i < n; ++i)
            accumulator[i] += recon[i];
    }

    // Average over spins
    for (int i = 0; i < n; ++i) accumulator[i] /= m_spins;

    bestThreshold /= (m_spins * qMax(1, m_levels));

    m_stats.signalLength = n;
    m_stats.decomposeLevels = m_levels;
    m_stats.threshold = bestThreshold;
    m_stats.inputSNR = 0.0;
    m_stats.outputSNR = estimateSNR(accumulator, signal);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit denoisingCompleted(n, bestThreshold, timer.elapsed());
    return accumulator;
}

/* ---- Reset ---- */

void WaveletDenoiser12::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
