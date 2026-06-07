/**
 * @file WaveletDenoiser6.cpp
 * @brief WaveletDenoiser6 实现
 *
 * 实现非抽取小波去噪：提升格式变换、循环旋转、软阈值。
 */

#include "utils/signal205/WaveletDenoiser6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WaveletDenoiser6::WaveletDenoiser6(QObject *parent) : QObject(parent) {}
WaveletDenoiser6::~WaveletDenoiser6() = default;

/* ---- Configuration ---- */

void WaveletDenoiser6::setWavelet(const QString& name) { m_wavelet = name; }
void WaveletDenoiser6::setDecompositionLevels(int levels) { m_levels = qMax(1, levels); }
void WaveletDenoiser6::setThresholdMethod(const QString& method) { m_thresholdMethod = method; }
void WaveletDenoiser6::setNumSpins(int spins) { m_numSpins = qMax(1, spins); }

/* ---- Circular shift ---- */

QVector<double> WaveletDenoiser6::circularShift(const QVector<double>& signal, int offset)
{
    int n = signal.size();
    if (n == 0) return {};
    QVector<double> shifted(n);
    for (int i = 0; i < n; ++i)
        shifted[i] = signal[((i - offset) % n + n) % n];
    return shifted;
}

/* ---- Compute power ---- */

double WaveletDenoiser6::computePower(const QVector<double>& signal)
{
    if (signal.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : signal) sum += s * s;
    return sum / signal.size();
}

/* ---- Predict step ---- */

void WaveletDenoiser6::predictStep(QVector<double>& detail, const QVector<double>& smooth)
{
    // Linear interpolation prediction
    int n = detail.size();
    for (int i = 0; i < n; ++i) {
        int i0 = i;
        int i1 = (i + 1) % smooth.size();
        detail[i] -= 0.5 * (smooth[i0] + smooth[i1]);
    }
}

/* ---- Update step ---- */

void WaveletDenoiser6::updateStep(QVector<double>& smooth, const QVector<double>& detail)
{
    int n = smooth.size();
    for (int i = 0; i < n; ++i) {
        int d0 = (i - 1 + detail.size()) % detail.size();
        smooth[i] += 0.25 * (detail[d0] + detail[i % detail.size()]);
    }
}

/* ---- Forward lifting (non-decimated) ---- */

QVector<QVector<double>> WaveletDenoiser6::forwardLifting(const QVector<double>& signal) const
{
    int n = signal.size();
    QVector<QVector<double>> coefficients;
    coefficients.reserve(m_levels + 1);

    QVector<double> current = signal;

    for (int level = 0; level < m_levels; ++level) {
        // Split: even/odd without downsampling
        int half = n / 2;
        QVector<double> smooth(n, 0.0);
        QVector<double> detail(n, 0.0);

        // Non-decimated: duplicate even to smooth, odd to detail
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) smooth[i] = current[i];
            else detail[i] = current[i];
        }

        // Predict
        predictStep(detail, smooth);

        // Update
        updateStep(smooth, detail);

        coefficients.append(detail);
        current = smooth;
    }
    coefficients.append(current); // approximation at last level
    return coefficients;
}

/* ---- Inverse lifting (non-decimated) ---- */

QVector<double> WaveletDenoiser6::inverseLifting(const QVector<QVector<double>>& coefficients) const
{
    if (coefficients.isEmpty()) return {};

    int n = coefficients[0].size();
    QVector<double> approx = coefficients.last();

    for (int level = coefficients.size() - 2; level >= 0; --level) {
        QVector<double> detail = coefficients[level];

        // Inverse update
        for (int i = 0; i < n; ++i) {
            int d0 = (i - 1 + detail.size()) % detail.size();
            approx[i] -= 0.25 * (detail[d0] + detail[i % detail.size()]);
        }

        // Inverse predict
        for (int i = 0; i < n; ++i) {
            int i0 = i;
            int i1 = (i + 1) % approx.size();
            detail[i] += 0.5 * (approx[i0] + approx[i1]);
        }

        // Merge
        for (int i = 0; i < n; ++i) {
            if (i % 2 != 0) approx[i] = detail[i];
        }
    }
    return approx;
}

/* ---- Soft threshold ---- */

QVector<double> WaveletDenoiser6::softThreshold(const QVector<double>& coeffs, double threshold) const
{
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i) {
        double sign = (coeffs[i] >= 0.0) ? 1.0 : -1.0;
        result[i] = sign * qMax(qAbs(coeffs[i]) - threshold, 0.0);
    }
    return result;
}

/* ---- Universal threshold ---- */

double WaveletDenoiser6::universalThreshold(const QVector<double>& coeffs) const
{
    int n = coeffs.size();
    if (n == 0) return 0.0;

    // Robust MAD estimate of noise standard deviation
    QVector<double> absCoeffs;
    absCoeffs.reserve(n);
    for (double c : coeffs) absCoeffs.append(qAbs(c));
    std::sort(absCoeffs.begin(), absCoeffs.end());

    double median = (n % 2 == 0) ? 0.5 * (absCoeffs[n / 2 - 1] + absCoeffs[n / 2])
                                  : absCoeffs[n / 2];
    double sigma = median / 0.6745; // MAD to std conversion

    return sigma * qSqrt(2.0 * qLn(n));
}

/* ---- Cycle-spin denoise ---- */

QVector<double> WaveletDenoiser6::cycleSpinDenoise(const QVector<double>& signal) const
{
    int n = signal.size();
    QVector<double> accumulated(n, 0.0);

    for (int spin = 0; spin < m_numSpins; ++spin) {
        int offset = spin * (n / m_numSpins);

        // Shift
        QVector<double> shifted = circularShift(signal, offset);

        // Forward transform
        QVector<QVector<double>> coeffs = forwardLifting(shifted);

        // Threshold detail coefficients
        for (int level = 0; level < coeffs.size() - 1; ++level) {
            double threshold = universalThreshold(coeffs[level]);
            coeffs[level] = softThreshold(coeffs[level], threshold);
        }

        // Inverse transform
        QVector<double> denoised = inverseLifting(coeffs);

        // Unshift
        QVector<double> unshifted = circularShift(denoised, -offset);

        // Accumulate
        for (int i = 0; i < n; ++i)
            accumulated[i] += unshifted[i];
    }

    // Average
    for (double& v : accumulated) v /= m_numSpins;
    return accumulated;
}

/* ---- Denoise ---- */

QVector<double> WaveletDenoiser6::denoise(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 4) return signal;

    double inPower = computePower(signal);
    QVector<double> result = cycleSpinDenoise(signal);
    double outPower = computePower(result);

    // Estimate noise power from residual
    double noisePower = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = signal[i] - result[i];
        noisePower += diff * diff;
    }
    noisePower /= n;

    m_stats.totalOps++;
    m_stats.signalLength = n;
    m_stats.numLevels = m_levels;
    m_stats.inputSNR = 10.0 * qLn(qMax(inPower, 1e-15) / qMax(noisePower, 1e-15)) / qLn(10.0);
    m_stats.outputSNR = 10.0 * qLn(qMax(outPower, 1e-15) / qMax(noisePower, 1e-15)) / qLn(10.0);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit denoisingCompleted(n, m_stats.inputSNR, m_stats.outputSNR, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void WaveletDenoiser6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
