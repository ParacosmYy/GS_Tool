/**
 * @file WaveletDenoiser20.cpp
 * @brief WaveletDenoiser20 实现
 *
 * 实现小波去噪器：SURE风险估计与自适应软硬阈值插值实现未知噪声水平最优去噪。
 */

#include "utils/signal303/WaveletDenoiser20.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser20::WaveletDenoiser20(QObject *parent)
    : QObject(parent) {}

WaveletDenoiser20::~WaveletDenoiser20() = default;

/* ---- Configuration ---- */

void WaveletDenoiser20::setWaveletType(WaveletType type) { m_wavelet = type; }
void WaveletDenoiser20::setDecompositionLevels(int levels) { m_maxLevels = qBound(1, levels, 20); }
void WaveletDenoiser20::setThresholdMethod(ThresholdMethod method) { m_method = method; }

/* ---- Low-pass filter coefficients ---- */

QVector<double> WaveletDenoiser20::lowPassFilter() const
{
    switch (m_wavelet) {
    case WaveletType::Haar:
        return {1.0 / qSqrt(2.0), 1.0 / qSqrt(2.0)};
    case WaveletType::Daubechies4: {
        double s = qSqrt(2.0);
        return {0.6830127/s, 1.1830127/s, 0.3169873/s, -0.1830127/s};
    }
    case WaveletType::Daubechies8: {
        double s = qSqrt(2.0);
        return {0.2303778/s, 0.7148466/s, 0.6308808/s, -0.0279838/s,
                -0.1870348/s, 0.0308414/s, 0.0328830/s, -0.0105974/s};
    }
    case WaveletType::Symlet4: {
        double s = qSqrt(2.0);
        return {-0.0757657/s, -0.0296355/s, 0.4976187/s, 0.8037388/s,
                0.2978578/s, -0.0992195/s, -0.0126039/s, 0.0322231/s};
    }
    default: return {1.0 / qSqrt(2.0), 1.0 / qSqrt(2.0)};
    }
}

/* ---- High-pass filter coefficients ---- */

QVector<double> WaveletDenoiser20::highPassFilter() const
{
    QVector<double> lp = lowPassFilter();
    int n = lp.size();
    QVector<double> hp(n);
    for (int i = 0; i < n; ++i)
        hp[i] = (i % 2 == 0) ? lp[n - 1 - i] : -lp[n - 1 - i];
    return hp;
}

/* ---- Convolution with periodic extension ---- */

QVector<double> WaveletDenoiser20::convolve(const QVector<double>& signal,
                                              const QVector<double>& filter) const
{
    int n = signal.size();
    int f = filter.size();
    QVector<double> result(n / 2);

    for (int i = 0; i < n / 2; ++i) {
        double sum = 0.0;
        for (int j = 0; j < f; ++j) {
            int idx = (2 * i + j) % n;
            sum += signal[idx] * filter[j];
        }
        result[i] = sum;
    }
    return result;
}

/* ---- Forward wavelet transform ---- */

QVector<QVector<double>> WaveletDenoiser20::forwardTransform(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < 2) return {signal};

    int levels = qMin(m_maxLevels, static_cast<int>(qLn(n) / qLn(2.0)));
    QVector<QVector<double>> coeffs(levels + 1);

    QVector<double> current = signal;

    // Pad to power of 2 if needed
    int padded = 1;
    while (padded < n) padded <<= 1;
    if (padded > n) {
        current.resize(padded, 0.0);
    }

    QVector<double> lp = lowPassFilter();
    QVector<double> hp = highPassFilter();

    for (int l = 0; l < levels; ++l) {
        coeffs[l] = convolve(current, hp);  // Detail coefficients
        current = convolve(current, lp);    // Approximation
    }
    coeffs[levels] = current;               // Final approximation

    return coeffs;
}

/* ---- Inverse wavelet transform ---- */

QVector<double> WaveletDenoiser20::inverseTransform(const QVector<QVector<double>>& coefficients) const
{
    int levels = coefficients.size() - 1;
    if (levels <= 0) return coefficients.isEmpty() ? QVector<double>() : coefficients[0];

    QVector<double> approx = coefficients[levels];
    int filterLen = lowPassFilter().size();

    for (int l = levels - 1; l >= 0; --l) {
        const QVector<double>& detail = coefficients[l];
        int n = approx.size() + detail.size();
        QVector<double> reconstructed(n, 0.0);

        // Upsample and add
        QVector<double> lp = lowPassFilter();
        QVector<double> hp = highPassFilter();

        // Upsample approximation
        for (int i = 0; i < approx.size(); ++i) {
            for (int j = 0; j < filterLen; ++j) {
                int idx = 2 * i + j;
                if (idx < n)
                    reconstructed[idx] += approx[i] * lp[j];
            }
        }

        // Upsample detail
        for (int i = 0; i < detail.size(); ++i) {
            for (int j = 0; j < filterLen; ++j) {
                int idx = 2 * i + j;
                if (idx < n)
                    reconstructed[idx] += detail[i] * hp[j];
            }
        }

        approx = reconstructed;
    }

    return approx;
}

/* ---- Estimate noise sigma via MAD ---- */

double WaveletDenoiser20::estimateSigma(const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;

    // Median Absolute Deviation of finest detail coefficients
    QVector<double> absCoeffs;
    absCoeffs.reserve(detailCoeffs.size());
    for (double c : detailCoeffs)
        absCoeffs.append(qAbs(c));

    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median = absCoeffs[absCoeffs.size() / 2];

    // sigma = MAD / 0.6745
    return median / 0.6745;
}

/* ---- Compute SURE risk ---- */

double WaveletDenoiser20::computeSURERisk(const QVector<double>& coeffs, double threshold) const
{
    int n = coeffs.size();
    if (n == 0) return 0.0;

    // SURE = n * sigma^2 + sum(min(|c_i|, lambda)^2 - 2*sigma^2 * I(|c_i| <= lambda))
    double sigma = estimateSigma(coeffs);
    double sigma2 = sigma * sigma;

    double sure = n * sigma2;
    for (int i = 0; i < n; ++i) {
        double ac = qAbs(coeffs[i]);
        double minVal = qMin(ac, threshold);
        sure += minVal * minVal - 2.0 * sigma2 * (ac <= threshold ? 1.0 : 0.0);
    }

    return sure / n;
}

/* ---- Find optimal threshold via SURE ---- */

double WaveletDenoiser20::findOptimalThreshold(const QVector<double>& coeffs) const
{
    if (coeffs.isEmpty()) return 0.0;

    // Search over candidate thresholds
    double bestThreshold = 0.0;
    double bestRisk = std::numeric_limits<double>::infinity();

    // Candidates: values from sorted absolute coefficients
    QVector<double> absCoeffs;
    for (double c : coeffs) absCoeffs.append(qAbs(c));
    std::sort(absCoeffs.begin(), absCoeffs.end());

    int step = qMax(1, absCoeffs.size() / 50);
    for (int i = 0; i < absCoeffs.size(); i += step) {
        double t = absCoeffs[i];
        double risk = computeSURERisk(coeffs, t);
        if (risk < bestRisk) {
            bestRisk = risk;
            bestThreshold = t;
        }
    }

    // Also try universal threshold: sigma * sqrt(2 * ln(n))
    double sigma = estimateSigma(coeffs);
    double universal = sigma * qSqrt(2.0 * qLn(qMax(2, coeffs.size())));
    double riskUniversal = computeSURERisk(coeffs, universal);
    if (riskUniversal < bestRisk) {
        bestRisk = riskUniversal;
        bestThreshold = universal;
    }

    return bestThreshold;
}

/* ---- Soft thresholding ---- */

QVector<double> WaveletDenoiser20::softThreshold(const QVector<double>& coeffs, double lambda) const
{
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) {
        double ac = qAbs(coeffs[i]);
        result[i] = (ac > lambda) ? qCopysign(ac - lambda, coeffs[i]) : 0.0;
    }
    return result;
}

/* ---- Hard thresholding ---- */

QVector<double> WaveletDenoiser20::hardThreshold(const QVector<double>& coeffs, double lambda) const
{
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i)
        result[i] = (qAbs(coeffs[i]) > lambda) ? coeffs[i] : 0.0;
    return result;
}

/* ---- Adaptive soft-hard interpolation ---- */

QVector<double> WaveletDenoiser20::adaptiveThreshold(const QVector<double>& coeffs, double lambda) const
{
    // Interpolation between soft and hard: mu controls blend
    // mu = 0: hard threshold, mu = 1: soft threshold
    double mu = 0.5;  // Default blend factor
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) {
        double ac = qAbs(coeffs[i]);
        if (ac <= lambda) {
            result[i] = 0.0;
        } else {
            double soft = qCopysign(ac - lambda, coeffs[i]);
            double hard = coeffs[i];
            result[i] = mu * soft + (1.0 - mu) * hard;
        }
    }
    return result;
}

/* ---- Main denoise ---- */

WaveletDenoiser20::DenoiseResult WaveletDenoiser20::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    DenoiseResult result;
    int n = signal.size();
    if (n < 4) {
        result.signal = signal;
        result.elapsedMs = timer.elapsed();
        return result;
    }

    // Forward wavelet transform
    QVector<QVector<double>> coeffs = forwardTransform(signal);
    int levels = coeffs.size() - 1;
    result.levelsUsed = levels;

    // Estimate noise level from finest detail coefficients
    result.estimatedSigma = estimateSigma(coeffs[0]);

    // Threshold each detail level
    double totalRisk = 0.0;
    for (int l = 0; l < levels; ++l) {
        double threshold;
        if (m_method == ThresholdMethod::SURE) {
            threshold = findOptimalThreshold(coeffs[l]);
            totalRisk += computeSURERisk(coeffs[l], threshold);
        } else {
            // Universal threshold scaled by level
            double levelSigma = estimateSigma(coeffs[l]);
            threshold = levelSigma * qSqrt(2.0 * qLn(qMax(2, coeffs[l].size())));
        }

        switch (m_method) {
        case ThresholdMethod::Soft:
            coeffs[l] = softThreshold(coeffs[l], threshold);
            break;
        case ThresholdMethod::Hard:
            coeffs[l] = hardThreshold(coeffs[l], threshold);
            break;
        case ThresholdMethod::Adaptive:
        case ThresholdMethod::SURE:
            coeffs[l] = adaptiveThreshold(coeffs[l], threshold);
            break;
        }
    }

    result.sureRisk = totalRisk / levels;

    // Inverse transform
    result.signal = inverseTransform(coeffs);

    // Estimate noise removed
    result.noiseEstimate.resize(n);
    int minLen = qMin(n, result.signal.size());
    for (int i = 0; i < minLen; ++i)
        result.noiseEstimate[i] = signal[i] - result.signal[i];

    result.elapsedMs = timer.elapsed();

    m_stats.totalDenoise++;
    m_stats.lastSignalLength = n;
    m_sigmaSum += result.estimatedSigma;
    m_stats.avgSigma = m_sigmaSum / m_stats.totalDenoise;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoise;

    emit denoiseDone(result.estimatedSigma, result.sureRisk, levels, result.elapsedMs);
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser20::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_sigmaSum = 0.0;
}
