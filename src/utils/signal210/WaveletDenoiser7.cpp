/**
 * @file WaveletDenoiser7.cpp
 * @brief WaveletDenoiser7 实现
 *
 * 实现小波降噪：Haar/D4小波变换、BayesShrink阈值、循环平移平移不变处理。
 */

#include "utils/signal210/WaveletDenoiser7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser7::WaveletDenoiser7(QObject *parent) : QObject(parent) {}
WaveletDenoiser7::~WaveletDenoiser7() = default;

/* ---- Configuration ---- */

void WaveletDenoiser7::setWaveletType(int type) { m_waveletType = (type == 1) ? 1 : 0; }
void WaveletDenoiser7::setDecompositionLevels(int levels) { m_numLevels = qMax(1, levels); }
void WaveletDenoiser7::setNumCycleSpins(int spins) { m_numSpins = qMax(1, spins); }
void WaveletDenoiser7::setThresholdMode(int mode) { m_thresholdMode = (mode == 1) ? 1 : 0; }

/* ---- Cycle shift ---- */

QVector<double> WaveletDenoiser7::cycleShift(const QVector<double>& signal, int offset)
{
    int n = signal.size();
    QVector<double> shifted(n);
    for (int i = 0; i < n; ++i)
        shifted[i] = signal[(i + offset) % n];
    return shifted;
}

QVector<double> WaveletDenoiser7::cycleUnshift(const QVector<double>& signal, int offset)
{
    int n = signal.size();
    QVector<double> unshifted(n);
    for (int i = 0; i < n; ++i)
        unshifted[(i + offset) % n] = signal[i];
    return unshifted;
}

/* ---- Haar forward step ---- */

void WaveletDenoiser7::haarForward(QVector<double>& approx, QVector<double>& detail,
                                      const QVector<double>& input)
{
    int n = input.size() / 2;
    approx.resize(n);
    detail.resize(n);
    double invSqrt2 = 1.0 / qSqrt(2.0);
    for (int i = 0; i < n; ++i) {
        approx[i] = (input[2*i] + input[2*i+1]) * invSqrt2;
        detail[i] = (input[2*i] - input[2*i+1]) * invSqrt2;
    }
}

/* ---- Haar inverse step ---- */

void WaveletDenoiser7::haarInverse(QVector<double>& output,
                                      const QVector<double>& approx,
                                      const QVector<double>& detail)
{
    int n = approx.size();
    output.resize(2 * n);
    double invSqrt2 = 1.0 / qSqrt(2.0);
    for (int i = 0; i < n; ++i) {
        output[2*i] = (approx[i] + detail[i]) * invSqrt2;
        output[2*i+1] = (approx[i] - detail[i]) * invSqrt2;
    }
}

/* ---- D4 forward step (Daubechies-4) ---- */

void WaveletDenoiser7::d4Forward(QVector<double>& approx, QVector<double>& detail,
                                    const QVector<double>& input)
{
    // Daubechies-4 scaling coefficients
    static const double h0 = 0.4829629131445341;
    static const double h1 = 0.8365163037378079;
    static const double h2 = 0.2241438680420134;
    static const double h3 = -0.1294095225512603;
    // Wavelet coefficients
    static const double g0 = h3;
    static const double g1 = -h2;
    static const double g2 = h1;
    static const double g3 = -h0;

    int n = input.size() / 2;
    approx.resize(n);
    detail.resize(n);
    int len = input.size();
    for (int i = 0; i < n; ++i) {
        int i0 = (2*i) % len;
        int i1 = (2*i+1) % len;
        int i2 = (2*i+2) % len;
        int i3 = (2*i+3) % len;
        approx[i] = h0*input[i0] + h1*input[i1] + h2*input[i2] + h3*input[i3];
        detail[i] = g0*input[i0] + g1*input[i1] + g2*input[i2] + g3*input[i3];
    }
}

/* ---- D4 inverse step ---- */

void WaveletDenoiser7::d4Inverse(QVector<double>& output,
                                    const QVector<double>& approx,
                                    const QVector<double>& detail)
{
    static const double h0 = 0.4829629131445341;
    static const double h1 = 0.8365163037378079;
    static const double h2 = 0.2241438680420134;
    static const double h3 = -0.1294095225512603;
    static const double g0 = h3, g1 = -h2, g2 = h1, g3 = -h0;

    int n = approx.size();
    output.resize(2 * n);
    for (int i = 0; i < n; ++i) {
        int prev = (i - 1 + n) % n;
        output[2*i]   = h2*approx[prev] + h1*detail[prev] + h0*approx[i] + h3*detail[i];
        output[2*i+1] = h3*approx[prev] - h0*detail[prev] + h1*approx[i] - h2*detail[i];
    }
}

/* ---- Forward wavelet transform ---- */

QVector<QVector<double>> WaveletDenoiser7::forwardWT(const QVector<double>& signal) const
{
    int n = signal.size();
    int levels = qMin(m_numLevels, static_cast<int>(qLn(n) / qLn(2)));
    QVector<QVector<double>> coeffs(levels + 1);

    QVector<double> current = signal;
    for (int lev = 0; lev < levels; ++lev) {
        QVector<double> approx, detail;
        if (m_waveletType == 0)
            haarForward(approx, detail, current);
        else
            d4Forward(approx, detail, current);
        coeffs[lev] = detail;  // Detail coefficients at level lev
        current = approx;
    }
    coeffs[levels] = current; // Final approximation
    return coeffs;
}

/* ---- Inverse wavelet transform ---- */

QVector<double> WaveletDenoiser7::inverseWT(const QVector<QVector<double>>& coeffs) const
{
    int levels = coeffs.size() - 1;
    QVector<double> current = coeffs[levels];

    for (int lev = levels - 1; lev >= 0; --lev) {
        QVector<double> output;
        if (m_waveletType == 0)
            haarInverse(output, current, coeffs[lev]);
        else
            d4Inverse(output, current, coeffs[lev]);
        current = output;
    }
    return current;
}

/* ---- Estimate noise standard deviation ---- */

double WaveletDenoiser7::estimateNoiseStd(const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;
    // MAD (Median Absolute Deviation) estimator
    QVector<double> absCoeffs(detailCoeffs.size());
    for (int i = 0; i < detailCoeffs.size(); ++i)
        absCoeffs[i] = qAbs(detailCoeffs[i]);
    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median = absCoeffs[absCoeffs.size() / 2];
    return median / 0.6745; // Normalize for Gaussian
}

/* ---- BayesShrink threshold ---- */

double WaveletDenoiser7::bayesShrinkThreshold(const QVector<double>& detailCoeffs) const
{
    if (detailCoeffs.isEmpty()) return 0.0;
    double sigma = estimateNoiseStd(detailCoeffs);

    // Compute signal variance
    double sum = 0.0;
    for (double c : detailCoeffs) sum += c * c;
    double variance = sum / detailCoeffs.size();

    // BayesShrink: sigma^2 / sqrt(max(variance - sigma^2, epsilon))
    double signalVar = qMax(variance - sigma * sigma, 1e-15);
    return sigma * sigma / qSqrt(signalVar);
}

/* ---- Soft thresholding ---- */

QVector<double> WaveletDenoiser7::softThreshold(const QVector<double>& coeffs, double t)
{
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) {
        if (coeffs[i] > t) result[i] = coeffs[i] - t;
        else if (coeffs[i] < -t) result[i] = coeffs[i] + t;
        else result[i] = 0.0;
    }
    return result;
}

/* ---- Hard thresholding ---- */

QVector<double> WaveletDenoiser7::hardThreshold(const QVector<double>& coeffs, double t)
{
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i)
        result[i] = (qAbs(coeffs[i]) > t) ? coeffs[i] : 0.0;
    return result;
}

/* ---- Denoise with cycle-spinning ---- */

QVector<double> WaveletDenoiser7::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();
    int n = signal.size();
    if (n < 4) return signal;

    int levels = qMin(m_numLevels, static_cast<int>(qLn(n) / qLn(2)));
    QVector<double> accumulated(n, 0.0);

    // Cycle-spinning: average over multiple circular shifts
    for (int spin = 0; spin < m_numSpins; ++spin) {
        int offset = spin * n / m_numSpins;
        auto shifted = cycleShift(signal, offset);

        // Forward WT
        auto coeffs = forwardWT(shifted);

        // Threshold each detail level with BayesShrink
        for (int lev = 0; lev < levels; ++lev) {
            double threshold = bayesShrinkThreshold(coeffs[lev]);
            if (m_thresholdMode == 0)
                coeffs[lev] = softThreshold(coeffs[lev], threshold);
            else
                coeffs[lev] = hardThreshold(coeffs[lev], threshold);
        }

        // Inverse WT
        auto denoised = inverseWT(coeffs);

        // Unshift and accumulate
        auto unshifted = cycleUnshift(denoised, offset);
        for (int i = 0; i < n; ++i)
            accumulated[i] += unshifted[i];
    }

    // Average
    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = accumulated[i] / m_numSpins;

    m_stats.signalLength = n;
    m_stats.numLevels = levels;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit denoisingCompleted(levels, m_numSpins, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
