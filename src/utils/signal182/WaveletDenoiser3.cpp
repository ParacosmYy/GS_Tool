/**
 * @file WaveletDenoiser3.cpp
 * @brief WaveletDenoiser3 实现
 *
 * 实现小波去噪：多级Haar/Daubechies分解、软/硬阈值、VisuShrink/BayesShrink、SNR估计。
 */

#include "utils/signal182/WaveletDenoiser3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser3::WaveletDenoiser3(QObject *parent) : QObject(parent) {}
WaveletDenoiser3::~WaveletDenoiser3() = default;

/* ---- Configuration ---- */

void WaveletDenoiser3::setDecomposeLevels(int levels) { m_levels = qMax(1, levels); }
void WaveletDenoiser3::setThresholdType(ThresholdType type) { m_threshType = type; }
void WaveletDenoiser3::setThresholdMethod(ThresholdMethod method) { m_threshMethod = method; }
void WaveletDenoiser3::setWaveletLength(int len) { m_waveletLen = qMax(2, len); }

/* ---- Haar filter coefficients ---- */

void WaveletDenoiser3::haarCoeffs(QVector<double>& lowDec, QVector<double>& highDec,
                                    QVector<double>& lowRec, QVector<double>& highRec) const
{
    // Haar: simplest wavelet, length 2
    double invSqrt2 = 1.0 / qSqrt(2.0);
    lowDec = {invSqrt2, invSqrt2};
    highDec = {invSqrt2, -invSqrt2};
    lowRec = {invSqrt2, invSqrt2};
    highRec = {-invSqrt2, invSqrt2};
}

/* ---- Single-level DWT ---- */

void WaveletDenoiser3::dwtLevel(const QVector<double>& in,
                                  QVector<double>& approx,
                                  QVector<double>& detail) const
{
    QVector<double> lowDec, highDec, lowRec, highRec;
    haarCoeffs(lowDec, highDec, lowRec, highRec);

    int n = in.size();
    int half = n / 2;
    approx.resize(half);
    detail.resize(half);

    for (int i = 0; i < half; ++i) {
        double a = 0.0, d = 0.0;
        for (int k = 0; k < lowDec.size(); ++k) {
            int idx = 2 * i + k;
            if (idx < n) {
                a += in[idx] * lowDec[k];
                d += in[idx] * highDec[k];
            }
        }
        approx[i] = a;
        detail[i] = d;
    }
}

/* ---- Single-level IDWT ---- */

QVector<double> WaveletDenoiser3::idwtLevel(const QVector<double>& approx,
                                               const QVector<double>& detail,
                                               int targetLen) const
{
    QVector<double> lowDec, highDec, lowRec, highRec;
    haarCoeffs(lowDec, highDec, lowRec, highRec);

    int half = approx.size();
    QVector<double> out(targetLen, 0.0);

    for (int i = 0; i < half; ++i) {
        for (int k = 0; k < lowRec.size(); ++k) {
            int idx = 2 * i + k;
            if (idx < targetLen) {
                out[idx] += approx[i] * lowRec[k] + detail[i] * highRec[k];
            }
        }
    }
    return out;
}

/* ---- Multi-level decompose ---- */

QVector<QVector<double>> WaveletDenoiser3::decompose(
    const QVector<double>& signal) const
{
    QVector<QVector<double>> coeffs;
    QVector<double> current = signal;

    for (int level = 0; level < m_levels; ++level) {
        if (current.size() < 4) break;
        QVector<double> approx, detail;
        dwtLevel(current, approx, detail);
        coeffs.append(detail); // Detail coefficients (high-freq)
        current = approx;       // Approximation for next level
    }
    coeffs.append(current); // Final approximation
    return coeffs;
}

/* ---- Multi-level reconstruct ---- */

QVector<double> WaveletDenoiser3::reconstruct(
    const QVector<QVector<double>>& coefficients) const
{
    if (coefficients.isEmpty()) return {};

    int numLevels = coefficients.size() - 1;
    QVector<double> current = coefficients.last(); // Final approximation

    for (int level = numLevels - 1; level >= 0; --level) {
        int targetLen = current.size() * 2;
        // Pad to match detail size if needed
        current = idwtLevel(current, coefficients[level], targetLen);
    }
    return current;
}

/* ---- Noise sigma estimation via MAD ---- */

double WaveletDenoiser3::estimateNoiseSigma(
    const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;

    // Median absolute deviation
    QVector<double> sorted = detailCoeffs;
    std::sort(sorted.begin(), sorted.end());
    double median = sorted[sorted.size() / 2];

    QVector<double> absDev(sorted.size());
    for (int i = 0; i < sorted.size(); ++i)
        absDev[i] = qAbs(sorted[i] - median);
    std::sort(absDev.begin(), absDev.end());
    double mad = absDev[absDev.size() / 2];

    // MAD to sigma conversion: sigma = MAD / 0.6745
    return mad / 0.6745;
}

/* ---- VisuShrink threshold ---- */

double WaveletDenoiser3::visuThreshold(int N, double sigma) const
{
    return sigma * qSqrt(2.0 * qLn(static_cast<double>(N)));
}

/* ---- BayesShrink threshold ---- */

double WaveletDenoiser3::bayesThreshold(double sigma,
                                           const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return 0.0;

    // Estimate signal variance
    double sum2 = 0.0;
    for (double c : coeffs) sum2 += c * c;
    double sigmaX2 = qMax(sum2 / coeffs.size() - sigma * sigma, 1e-15);
    double sigmaX = qSqrt(sigmaX2);

    return sigma * sigma / sigmaX;
}

/* ---- Apply threshold ---- */

QVector<double> WaveletDenoiser3::applyThreshold(const QVector<double>& coeffs,
                                                    double T) const
{
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) {
        if (m_threshType == Soft) {
            // Soft: sign(x) * max(|x| - T, 0)
            if (coeffs[i] > T) result[i] = coeffs[i] - T;
            else if (coeffs[i] < -T) result[i] = coeffs[i] + T;
            else result[i] = 0.0;
        } else {
            // Hard: x if |x| > T, else 0
            result[i] = (qAbs(coeffs[i]) > T) ? coeffs[i] : 0.0;
        }
    }
    return result;
}

/* ---- SNR estimation ---- */

double WaveletDenoiser3::estimateSNR(const QVector<double>& signal,
                                       const QVector<double>& noise) const
{
    double sigPow = 0.0, noisePow = 0.0;
    int n = qMin(signal.size(), noise.size());
    for (int i = 0; i < n; ++i) {
        sigPow += signal[i] * signal[i];
        noisePow += noise[i] * noise[i];
    }
    if (noisePow < 1e-30) return 100.0; // Very high SNR
    return 10.0 * qLog10(sigPow / noisePow);
}

/* ---- Main denoise ---- */

QVector<double> WaveletDenoiser3::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    // Decompose
    auto coeffs = decompose(signal);
    int numLevels = coeffs.size() - 1;
    if (numLevels == 0) return signal;

    // Estimate noise from finest detail coefficients
    double sigma = estimateNoiseSigma(coeffs[0]);

    // Apply threshold to each detail level
    for (int level = 0; level < numLevels; ++level) {
        double T;
        switch (m_threshMethod) {
        case VisuShrink:
            T = visuThreshold(n, sigma);
            break;
        case BayesShrink:
            T = bayesThreshold(sigma, coeffs[level]);
            break;
        case Minimax:
            // Minimax approximation
            T = sigma * (0.3936 + 0.1829 * qLn(static_cast<double>(n)));
            break;
        default:
            T = visuThreshold(n, sigma);
            break;
        }
        coeffs[level] = applyThreshold(coeffs[level], T);
    }

    // Reconstruct
    QVector<double> denoised = reconstruct(coeffs);

    // Estimate SNR improvement
    QVector<double> noise(n);
    int minLen = qMin(n, denoised.size());
    for (int i = 0; i < minLen; ++i)
        noise[i] = signal[i] - denoised[i];
    // Pad if sizes differ
    denoised.resize(n);

    double inSNR = estimateSNR(signal, noise);
    double outSNR = estimateSNR(denoised, noise);
    double usedThreshold = (m_threshMethod == BayesShrink)
        ? bayesThreshold(sigma, coeffs[0])
        : visuThreshold(n, sigma);

    m_stats.totalDenoiseOps++;
    m_stats.signalLength = n;
    m_stats.decomposeLevels = numLevels;
    m_stats.inputSNR = inSNR;
    m_stats.outputSNR = outSNR;
    m_stats.threshold = usedThreshold;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoiseOps;

    emit denoiseCompleted(inSNR, outSNR, usedThreshold);
    return denoised;
}

/* ---- Reset ---- */

void WaveletDenoiser3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
