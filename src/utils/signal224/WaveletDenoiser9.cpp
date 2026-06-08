/**
 * @file WaveletDenoiser9.cpp
 * @brief WaveletDenoiser9 实现
 *
 * 实现小波去噪：SWT平稳小波变换、SureShrink层依赖阈值。
 */

#include "utils/signal224/WaveletDenoiser9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser9::WaveletDenoiser9(QObject *parent) : QObject(parent) {}
WaveletDenoiser9::~WaveletDenoiser9() = default;

/* ---- Configuration ---- */

void WaveletDenoiser9::setParameters(Wavelet wavelet, int levels)
{
    m_wavelet = wavelet;
    m_levels = levels;
    loadFilters();
}

/* ---- Load filter coefficients ---- */

void WaveletDenoiser9::loadFilters()
{
    switch (m_wavelet) {
    case Wavelet::Haar:
        m_loD = {0.7071067811865476, 0.7071067811865476};
        m_hiD = {0.7071067811865476, -0.7071067811865476};
        m_loR = {0.7071067811865476, 0.7071067811865476};
        m_hiR = {0.7071067811865476, -0.7071067811865476};
        break;
    case Wavelet::DB2:
        m_loD = {-0.1294095225509214, 0.2241438680418573,
                  0.836516303737469, 0.4829629131446906};
        m_hiD = {-0.4829629131446906, 0.836516303737469,
                  -0.2241438680418573, -0.1294095225509214};
        m_loR = {0.4829629131446906, 0.836516303737469,
                  0.2241438680418573, -0.1294095225509214};
        m_hiR = {-0.1294095225509214, -0.2241438680418573,
                  0.836516303737469, -0.4829629131446906};
        break;
    case Wavelet::DB4:
        m_loD = {-0.0105974017849973, 0.0328830116669829, 0.0308413818359870,
                  -0.1870348117188812, -0.0279837694169838, 0.6308807679295904,
                  0.7148465705525415, 0.2303778133088552};
        m_hiD = {-0.2303778133088552, 0.7148465705525415, -0.6308807679295904,
                  -0.0279837694169838, 0.1870348117188812, 0.0308413818359870,
                  -0.0328830116669829, -0.0105974017849973};
        m_loR = {0.2303778133088552, 0.7148465705525415, 0.6308807679295904,
                  -0.0279837694169838, -0.1870348117188812, 0.0308413818359870,
                  0.0328830116669829, -0.0105974017849973};
        m_hiR = {-0.0105974017849973, -0.0328830116669829, 0.0308413818359870,
                  0.1870348117188812, -0.0279837694169838, -0.6308807679295904,
                  0.7148465705525415, -0.2303778133088552};
        break;
    case Wavelet::Sym4:
        m_loD = {-0.0757657147892733, -0.0296355276459985, 0.4976186676324553,
                  0.8037387518059161, 0.2978577956052774, -0.0992195435768472,
                  -0.0126039672620378, 0.0322231006040427};
        m_hiD = {-0.0322231006040427, -0.0126039672620378, 0.0992195435768472,
                  0.2978577956052774, -0.8037387518059161, 0.4976186676324553,
                  0.0296355276459985, -0.0757657147892733};
        m_loR = {0.0322231006040427, -0.0126039672620378, -0.0992195435768472,
                  0.2978577956052774, 0.8037387518059161, 0.4976186676324553,
                  -0.0296355276459985, -0.0757657147892733};
        m_hiR = {-0.0757657147892733, 0.0296355276459985, 0.4976186676324553,
                  -0.8037387518059161, 0.2978577956052774, 0.0992195435768472,
                  -0.0126039672620378, -0.0322231006040427};
        break;
    }
}

/* ---- Circular convolution ---- */

QVector<double> WaveletDenoiser9::circConvolve(const QVector<double>& signal,
    const QVector<double>& filter) const
{
    int n = signal.size();
    int f = filter.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int k = 0; k < f; ++k) {
            int idx = (i - k + n) % n;
            s += filter[k] * signal[idx];
        }
        result[i] = s;
    }
    return result;
}

/* ---- Forward SWT ---- */

void WaveletDenoiser9::forwardSWT(const QVector<double>& signal,
                                    QVector<QVector<double>>& approx,
                                    QVector<QVector<double>>& detail) const
{
    int n = signal.size();
    int levels = (m_levels < 1) ? qFloor(qLn(n) / qLn(2.0)) : m_levels;
    levels = qBound(1, levels, qFloor(qLn(n) / qLn(2.0)));

    approx.resize(levels);
    detail.resize(levels);

    QVector<double> current = signal;
    for (int lev = 0; lev < levels; ++lev) {
        // SWT: upsample filters at each level
        int step = 1 << lev;
        QVector<double> loUp, hiUp;
        for (int i = 0; i < m_loD.size(); ++i) {
            loUp.append(m_loD[i]);
            for (int s = 1; s < step; ++s) loUp.append(0.0);
            hiUp.append(m_hiD[i]);
            for (int s = 1; s < step; ++s) hiUp.append(0.0);
        }
        // Pad filter to signal length
        while (loUp.size() < n) loUp.append(0.0);
        while (hiUp.size() < n) hiUp.append(0.0);
        loUp.resize(n);
        hiUp.resize(n);

        detail[lev] = circConvolve(current, hiUp);
        approx[lev] = circConvolve(current, loUp);
        current = approx[lev];
    }
}

/* ---- Inverse SWT ---- */

QVector<double> WaveletDenoiser9::inverseSWT(
    const QVector<QVector<double>>& approx,
    const QVector<QVector<double>>& detail) const
{
    int levels = qMin(approx.size(), detail.size());
    if (levels == 0) return {};

    QVector<double> current = approx[levels - 1];
    int n = current.size();

    for (int lev = levels - 1; lev >= 0; --lev) {
        int step = 1 << lev;
        QVector<double> loUp, hiUp;
        for (int i = 0; i < m_loR.size(); ++i) {
            loUp.append(m_loR[i]);
            for (int s = 1; s < step; ++s) loUp.append(0.0);
            hiUp.append(m_hiR[i]);
            for (int s = 1; s < step; ++s) hiUp.append(0.0);
        }
        while (loUp.size() < n) loUp.append(0.0);
        while (hiUp.size() < n) hiUp.append(0.0);
        loUp.resize(n);
        hiUp.resize(n);

        QVector<double> aConv = circConvolve(current, loUp);
        QVector<double> dConv = circConvolve(detail[lev], hiUp);

        for (int i = 0; i < n; ++i)
            current[i] = aConv[i] + dConv[i];
    }
    return current;
}

/* ---- Soft thresholding ---- */

double WaveletDenoiser9::softThreshold(double value, double threshold)
{
    if (value > threshold) return value - threshold;
    if (value < -threshold) return value + threshold;
    return 0.0;
}

/* ---- Compute SURE ---- */

double WaveletDenoiser9::computeSURE(const QVector<double>& coeffs,
                                       double threshold) const
{
    int n = coeffs.size();
    if (n == 0) return 0.0;
    double sumSq = 0.0;
    int countBelow = 0;
    for (int i = 0; i < n; ++i) {
        double c = coeffs[i];
        if (qAbs(c) <= threshold) {
            sumSq += c * c;
            countBelow++;
        }
    }
    return n - 2.0 * countBelow + sumSq / qMax(threshold * threshold, 1e-30);
}

/* ---- SureShrink threshold ---- */

double WaveletDenoiser9::sureShrinkThreshold(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return 0.0;

    // Get sorted absolute values
    QVector<double> sorted;
    for (double c : coeffs) sorted.append(qAbs(c));
    std::sort(sorted.begin(), sorted.end());

    // Find threshold minimizing SURE
    double bestT = sorted.last();
    double bestSURE = std::numeric_limits<double>::max();

    for (int i = 0; i < sorted.size(); ++i) {
        double t = sorted[i];
        double sure = computeSURE(coeffs, t);
        if (sure < bestSURE) { bestSURE = sure; bestT = t; }
    }

    // Compare with universal threshold
    double uniT = universalThreshold(coeffs, coeffs.size());
    if (computeSURE(coeffs, uniT) < bestSURE) bestT = uniT;

    return bestT;
}

/* ---- Universal threshold ---- */

double WaveletDenoiser9::universalThreshold(const QVector<double>& coeffs,
                                               int n) const
{
    if (coeffs.isEmpty()) return 0.0;
    double sumSq = 0.0;
    for (double c : coeffs) sumSq += c * c;
    double sigma = qSqrt(sumSq / n) / qSqrt(qLn(2.0 * qLn(n + 1)));
    return sigma * qSqrt(2.0 * qLn(n));
}

/* ---- Compute SNR ---- */

double WaveletDenoiser9::computeSNR(const QVector<double>& signal,
                                      const QVector<double>& noise)
{
    double sigPow = 0.0, noisePow = 0.0;
    for (int i = 0; i < signal.size(); ++i) sigPow += signal[i] * signal[i];
    for (int i = 0; i < noise.size(); ++i) noisePow += noise[i] * noise[i];
    if (noisePow < 1e-30) return 100.0;
    return 10.0 * qLn(sigPow / noisePow) / qLn(10.0);
}

/* ---- Denoise ---- */

WaveletDenoiser9::DenoiseResult WaveletDenoiser9::denoise(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    DenoiseResult result;
    int n = signal.size();
    if (n < 4) { result.signal = signal; return result; }

    m_stats.signalLength = n;

    // Forward SWT
    QVector<QVector<double>> approx, detail;
    forwardSWT(signal, approx, detail);

    int levels = detail.size();
    m_stats.decompLevels = levels;
    result.levels = levels;

    // Estimate noise level from finest detail coefficients
    double sigma = 0.0;
    if (!detail.isEmpty()) {
        QVector<double> absCoeffs;
        for (double c : detail[0]) absCoeffs.append(qAbs(c));
        std::sort(absCoeffs.begin(), absCoeffs.end());
        sigma = absCoeffs[absCoeffs.size() / 2] / 0.6745;
    }

    // Apply SureShrink threshold at each level
    result.thresholds.resize(levels);
    QVector<QVector<double>> denoisedDetail = detail;
    for (int lev = 0; lev < levels; ++lev) {
        double t = sureShrinkThreshold(detail[lev]);
        result.thresholds[lev] = t;
        for (int i = 0; i < denoisedDetail[lev].size(); ++i)
            denoisedDetail[lev][i] = softThreshold(detail[lev][i], t);
    }

    // Reconstruct
    result.signal = inverseSWT(approx, denoisedDetail);

    // Compute noise residual
    result.noise.resize(n);
    for (int i = 0; i < n && i < result.signal.size(); ++i)
        result.noise[i] = signal[i] - result.signal[i];

    result.inputSNR = 0.0;
    result.outputSNR = computeSNR(result.signal, result.noise);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit denoisingCompleted(levels, result.inputSNR, result.outputSNR,
                             timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_loD.clear(); m_hiD.clear();
    m_loR.clear(); m_hiR.clear();
}
