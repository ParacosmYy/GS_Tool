/**
 * @file WaveletDenoiser19.cpp
 * @brief WaveletDenoiser19 实现
 *
 * 实现小波去噪器：循环平移与平移不变阈值实现减少吉布斯伪影信号恢复。
 */

#include "utils/signal294/WaveletDenoiser19.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WaveletDenoiser19::WaveletDenoiser19(QObject *parent)
    : QObject(parent) {}

WaveletDenoiser19::~WaveletDenoiser19() = default;

/* ---- Configuration ---- */

void WaveletDenoiser19::setWavelet(const QString& name) { m_waveletName = name.toLower(); }
void WaveletDenoiser19::setDecompositionLevels(int levels) { m_levels = qBound(1, levels, 20); }
void WaveletDenoiser19::setThresholdMethod(ThresholdMethod method) { m_thresholdMethod = method; }
void WaveletDenoiser19::setNumCycleSpins(int spins) { m_numSpins = qBound(1, spins, 64); }
void WaveletDenoiser19::setManualThreshold(double t) { m_manualThreshold = qBound(0.0, t, 1e10); }

/* ---- Estimate noise sigma via MAD ---- */

double WaveletDenoiser19::estimateNoiseSigma(const QVector<double>& signal) const
{
    int n = signal.size();
    if (n < 2) return 0.0;

    // Compute finest-level detail coefficients (differences)
    QVector<double> diff;
    int half = n / 2;
    diff.reserve(half);
    for (int i = 0; i + 1 < n; i += 2)
        diff.append((signal[i] - signal[i + 1]) / qSqrt(2.0));

    if (diff.isEmpty()) return 0.0;

    // MAD (median absolute deviation)
    QVector<double> absDiff = diff;
    for (auto& v : absDiff) v = qAbs(v);
    std::sort(absDiff.begin(), absDiff.end());
    double median = absDiff[absDiff.size() / 2];
    return median / 0.6745; // Scale factor for normal distribution
}

/* ---- Universal threshold ---- */

double WaveletDenoiser19::universalThreshold(double sigma, int n) const
{
    return sigma * qSqrt(2.0 * qLn(qMax(n, 1)));
}

/* ---- Haar forward (1 level) ---- */

void WaveletDenoiser19::haarForward(const QVector<double>& in,
                                       QVector<double>& approx,
                                       QVector<double>& detail) const
{
    int n = in.size();
    int half = n / 2;
    approx.resize(half);
    detail.resize(half);
    double invSqrt2 = 1.0 / qSqrt(2.0);

    for (int i = 0; i < half; ++i) {
        approx[i] = (in[2 * i] + in[2 * i + 1]) * invSqrt2;
        detail[i] = (in[2 * i] - in[2 * i + 1]) * invSqrt2;
    }
}

/* ---- Haar inverse (1 level) ---- */

void WaveletDenoiser19::haarInverse(const QVector<double>& approx,
                                       const QVector<double>& detail,
                                       QVector<double>& out) const
{
    int half = approx.size();
    out.resize(half * 2);
    double invSqrt2 = 1.0 / qSqrt(2.0);

    for (int i = 0; i < half; ++i) {
        out[2 * i] = (approx[i] + detail[i]) * invSqrt2;
        out[2 * i + 1] = (approx[i] - detail[i]) * invSqrt2;
    }
}

/* ---- Multi-level forward WT ---- */

void WaveletDenoiser19::forwardWT(const QVector<double>& signal,
                                    QVector<QVector<double>>& details,
                                    QVector<double>& finalApprox) const
{
    details.clear();
    QVector<double> current = signal;

    for (int lev = 0; lev < m_levels; ++lev) {
        if (current.size() < 2) break;
        QVector<double> a, d;
        haarForward(current, a, d);
        details.append(d);
        current = a;
    }
    finalApprox = current;
}

/* ---- Multi-level inverse WT ---- */

QVector<double> WaveletDenoiser19::inverseWT(
    const QVector<QVector<double>>& details,
    const QVector<double>& finalApprox) const
{
    QVector<double> current = finalApprox;
    for (int lev = details.size() - 1; lev >= 0; --lev) {
        QVector<double> reconstructed;
        haarInverse(current, details[lev], reconstructed);
        current = reconstructed;
    }
    return current;
}

/* ---- Apply threshold ---- */

void WaveletDenoiser19::applyThreshold(QVector<double>& coeffs, double threshold) const
{
    for (auto& c : coeffs) {
        double absC = qAbs(c);
        if (absC <= threshold) {
            c = 0.0;
        } else {
            switch (m_thresholdMethod) {
            case SoftThreshold:
                c = (c > 0) ? (c - threshold) : (c + threshold);
                break;
            case HardThreshold:
                break; // Keep as is
            case GarroteThreshold:
                c = c - threshold * threshold / c;
                break;
            }
        }
    }
}

/* ---- Cycle shift ---- */

QVector<double> WaveletDenoiser19::cycleShift(const QVector<double>& signal, int k) const
{
    int n = signal.size();
    QVector<double> shifted(n);
    for (int i = 0; i < n; ++i)
        shifted[i] = signal[(i + k) % n];
    return shifted;
}

/* ---- Cycle unshift ---- */

QVector<double> WaveletDenoiser19::cycleUnshift(const QVector<double>& signal, int k, int origLen) const
{
    int n = qMin(signal.size(), origLen);
    QVector<double> unshifted(n);
    for (int i = 0; i < n; ++i)
        unshifted[i] = signal[((i - k) % origLen + origLen) % origLen];
    return unshifted;
}

/* ---- Main denoise ---- */

WaveletDenoiser19::DenoiseResult WaveletDenoiser19::denoise(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    DenoiseResult result;
    int n = signal.size();
    if (n < 4) {
        result.signal = signal;
        return result;
    }

    // Estimate noise level
    double sigma = estimateNoiseSigma(signal);
    result.noiseEstimate = sigma;

    // Determine threshold
    double threshold = (m_manualThreshold > 0.0)
                           ? m_manualThreshold
                           : universalThreshold(sigma, n);

    // Cycle-spinning for translation-invariant denoising
    QVector<double> accumulated(n, 0.0);

    for (int spin = 0; spin < m_numSpins; ++spin) {
        int shift = spin; // Each spin uses different shift amount

        // Shift signal
        auto shifted = cycleShift(signal, shift);

        // Pad to power of 2 for clean decomposition
        int paddedLen = 1;
        while (paddedLen < shifted.size()) paddedLen *= 2;
        QVector<double> padded(paddedLen, 0.0);
        for (int i = 0; i < shifted.size(); ++i)
            padded[i] = shifted[i];

        // Forward wavelet transform
        QVector<QVector<double>> details;
        QVector<double> finalApprox;
        forwardWT(padded, details, finalApprox);

        // Count zeroed coefficients
        for (const auto& d : details)
            for (double c : d)
                if (qAbs(c) <= threshold)
                    result.numCoeffsZeroed++;

        // Apply thresholding to detail coefficients only
        for (auto& d : details)
            applyThreshold(d, threshold);

        // Inverse wavelet transform
        auto reconstructed = inverseWT(details, finalApprox);

        // Unshift
        auto unshifted = cycleUnshift(reconstructed, shift, n);

        // Accumulate
        for (int i = 0; i < n; ++i)
            accumulated[i] += unshifted[i];
    }

    // Average over all spins
    result.signal.resize(n);
    for (int i = 0; i < n; ++i)
        result.signal[i] = accumulated[i] / m_numSpins;

    // Compute SNR estimates
    double signalPower = 0.0, noisePower = 0.0;
    for (int i = 0; i < n; ++i) {
        signalPower += signal[i] * signal[i];
        double noise = signal[i] - result.signal[i];
        noisePower += noise * noise;
    }
    result.inputSNR = (noisePower > 1e-30)
                          ? 10.0 * qLn(signalPower / noisePower) / qLn(10.0)
                          : 100.0;

    double residualPower = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = signal[i] - result.signal[i];
        residualPower += r * r;
    }
    result.outputSNR = (residualPower > 1e-30 && signalPower > 1e-30)
                           ? 10.0 * qLn(signalPower / residualPower) / qLn(10.0)
                           : 100.0;

    double elapsed = timer.elapsed();
    m_stats.signalLength = n;
    m_stats.numScales = m_levels;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit denoiseDone(n, sigma, elapsed);
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser19::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
