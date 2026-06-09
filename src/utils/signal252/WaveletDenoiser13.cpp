/**
 * @file WaveletDenoiser13.cpp
 * @brief WaveletDenoiser13 实现
 *
 * 实现小波去噪：自适应贝叶斯软阈值与尺度间父子依赖建模。
 */

#include "utils/signal252/WaveletDenoiser13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser13::WaveletDenoiser13(QObject *parent) : QObject(parent) {}
WaveletDenoiser13::~WaveletDenoiser13() = default;

/* ---- Configuration ---- */

void WaveletDenoiser13::setNumLevels(int levels)
{
    m_numLevels = qMax(1, levels);
}
void WaveletDenoiser13::setWavelet(const QString& wavelet)
{
    m_wavelet = wavelet.toLower();
}

/* ---- Get wavelet filter coefficients ---- */

void WaveletDenoiser13::getFilterCoeffs(
    QVector<double>& loD, QVector<double>& hiD,
    QVector<double>& loR, QVector<double>& hiR) const
{
    if (m_wavelet == "db2") {
        // Daubechies 2 (4-tap)
        double s = 1.0 / qSqrt(2.0);
        loD = {0.482962913 * s, 0.836516304 * s, 0.224143868 * s, -0.129409523 * s};
    } else if (m_wavelet == "db4") {
        // Daubechies 4 (8-tap)
        double sq2 = 1.0 / qSqrt(2.0);
        loD = {
            0.230377813309 * sq2, 0.714846570553 * sq2,
            0.630880767930 * sq2, -0.027983769417 * sq2,
            -0.187034811719 * sq2, 0.030841381836 * sq2,
            0.032883011667 * sq2, -0.010597401785 * sq2
        };
    } else {
        // Haar (2-tap)
        double s = 1.0 / qSqrt(2.0);
        loD = {s, s};
    }

    int n = loD.size();
    hiD.resize(n);
    loR.resize(n);
    hiR.resize(n);

    // High-pass decomposition: alternating flip of low-pass
    for (int i = 0; i < n; ++i)
        hiD[i] = (i % 2 == 0 ? 1.0 : -1.0) * loD[n - 1 - i];

    // Reconstruction filters: time-reversed
    for (int i = 0; i < n; ++i) {
        loR[i] = loD[n - 1 - i];
        hiR[i] = hiD[n - 1 - i];
    }
}

/* ---- Downsample-convolve ---- */

QVector<double> WaveletDenoiser13::downsampleConvolve(
    const QVector<double>& sig, const QVector<double>& filter) const
{
    int n = sig.size();
    int fLen = filter.size();
    int outLen = (n + fLen - 1) / 2;
    QVector<double> out(outLen, 0.0);
    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int k = 0; k < fLen; ++k) {
            int idx = 2 * i + k;
            if (idx < n) sum += filter[k] * sig[idx];
        }
        out[i] = sum;
    }
    return out;
}

/* ---- Upsample-convolve ---- */

QVector<double> WaveletDenoiser13::upsampleConvolve(
    const QVector<double>& sig, const QVector<double>& filter,
    int targetLen) const
{
    int fLen = filter.size();
    QVector<double> out(targetLen, 0.0);
    for (int i = 0; i < sig.size(); ++i) {
        for (int k = 0; k < fLen; ++k) {
            int idx = 2 * i + k;
            if (idx < targetLen) out[idx] += filter[k] * sig[i];
        }
    }
    return out;
}

/* ---- Wavelet decomposition ---- */

void WaveletDenoiser13::decompose(const QVector<double>& signal)
{
    QVector<double> loD, hiD, loR, hiR;
    getFilterCoeffs(loD, hiD, loR, hiR);

    m_coeffs.clear();
    m_coeffs.resize(m_numLevels);

    QVector<double> current = signal;
    for (int lev = 0; lev < m_numLevels; ++lev) {
        auto approx = downsampleConvolve(current, loD);
        m_coeffs[lev] = downsampleConvolve(current, hiD);
        current = approx;
    }
    m_approx = current;
}

/* ---- Estimate noise sigma via MAD ---- */

double WaveletDenoiser13::estimateNoiseSigma(
    const QVector<double>& detail) const
{
    if (detail.isEmpty()) return 0.0;
    QVector<double> absVals;
    absVals.reserve(detail.size());
    for (double v : detail) absVals.append(qAbs(v));
    std::sort(absVals.begin(), absVals.end());
    double median = absVals[absVals.size() / 2];
    return median / 0.6745;
}

/* ---- Bayesian threshold estimation ---- */

double WaveletDenoiser13::bayesianThreshold(
    const QVector<double>& detail, double sigma) const
{
    if (detail.isEmpty() || sigma < 1e-15) return 0.0;

    // Bayesian estimate: threshold = sigma^2 / sigma_x
    // where sigma_x^2 = max(0, var(detail) - sigma^2)
    double sum = 0.0;
    for (double v : detail) sum += v * v;
    double varX = sum / detail.size();
    double sigmaX2 = qMax(0.0, varX - sigma * sigma);
    double sigmaX = qSqrt(sigmaX2);

    if (sigmaX < 1e-15) return qSqrt(2.0 * qLn(qMax(1, detail.size()))) * sigma;

    // Bayesian soft threshold: T = sigma^2 / sigma_x
    return sigma * sigma / sigmaX;
}

/* ---- Soft thresholding ---- */

void WaveletDenoiser13::softThreshold(QVector<double>& coeff, double threshold)
{
    for (auto& v : coeff) {
        if (v > threshold) v -= threshold;
        else if (v < -threshold) v += threshold;
        else v = 0.0;
    }
}

/* ---- Interscale parent-child dependency modeling ---- */

void WaveletDenoiser13::applyParentChildDependency()
{
    for (int lev = m_numLevels - 1; lev >= 1; --lev) {
        int parentLen = m_coeffs[lev].size();
        int childLen = m_coeffs[lev - 1].size();
        for (int i = 0; i < childLen; ++i) {
            int parentIdx = i / 2;
            if (parentIdx >= parentLen) continue;

            double parent = m_coeffs[lev][parentIdx];
            double child = m_coeffs[lev - 1][i];

            // If parent is significant, child is more likely significant
            // Scale child by parent magnitude (dependency model)
            double parentMag = qAbs(parent);
            double childMag = qAbs(child);

            if (parentMag > m_thresholds[lev] && childMag < m_thresholds[lev - 1]) {
                // Parent significant, child not: boost child slightly
                double boost = 0.5 * (parentMag - m_thresholds[lev]) /
                               qMax(parentMag, 1e-15);
                m_coeffs[lev - 1][i] = child * (1.0 + boost);
            } else if (parentMag < m_thresholds[lev] && childMag > m_thresholds[lev - 1]) {
                // Parent insignificant, child significant: attenuate child
                double atten = qMax(0.3, parentMag / qMax(m_thresholds[lev], 1e-15));
                m_coeffs[lev - 1][i] = child * atten;
            }
        }
    }
}

/* ---- Wavelet reconstruction ---- */

QVector<double> WaveletDenoiser13::reconstruct()
{
    QVector<double> loD, hiD, loR, hiR;
    getFilterCoeffs(loD, hiD, loR, hiR);

    QVector<double> current = m_approx;
    for (int lev = m_numLevels - 1; lev >= 0; --lev) {
        int targetLen = current.size() * 2;
        auto upApprox = upsampleConvolve(current, loR, targetLen);
        auto upDetail = upsampleConvolve(m_coeffs[lev], hiR, targetLen);
        current.resize(targetLen);
        for (int i = 0; i < targetLen; ++i)
            current[i] = upApprox[i] + upDetail[i];
    }
    return current;
}

/* ---- Main denoise ---- */

QVector<double> WaveletDenoiser13::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    // Decompose
    decompose(signal);

    // Estimate noise from finest detail coefficients
    double sigma = estimateNoiseSigma(m_coeffs[0]);

    // Compute Bayesian thresholds per level and apply
    m_thresholds.resize(m_numLevels);
    for (int lev = 0; lev < m_numLevels; ++lev) {
        m_thresholds[lev] = bayesianThreshold(m_coeffs[lev], sigma);
        softThreshold(m_coeffs[lev], m_thresholds[lev]);
    }

    // Apply interscale parent-child dependency
    applyParentChildDependency();

    // Reconstruct
    auto result = reconstruct();

    // Truncate to original length
    if (result.size() > n) result.resize(n);

    // Compute SNR improvement
    double signalPower = 0.0, noisePower = 0.0;
    for (int i = 0; i < n; ++i) {
        signalPower += signal[i] * signal[i];
        double diff = signal[i] - (i < result.size() ? result[i] : 0.0);
        noisePower += diff * diff;
    }
    double inputSNR = (signalPower > 0) ? 10.0 * qLn(signalPower / qMax(noisePower, 1e-30)) / qLn(10.0) : 0.0;

    m_stats.signalLength = n;
    m_stats.numLevels = m_numLevels;
    m_stats.noiseSigma = sigma;
    m_stats.inputSNR = inputSNR;
    m_stats.outputSNR = inputSNR;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit denoisingCompleted(n, m_stats.outputSNR, timer.elapsed());
    return result;
}

/* ---- Get coefficients ---- */

QVector<QVector<double>> WaveletDenoiser13::coefficients() const
{
    return m_coeffs;
}

/* ---- Reset ---- */

void WaveletDenoiser13::resetStatistics()
{
    m_coeffs.clear();
    m_approx.clear();
    m_thresholds.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
