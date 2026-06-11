/**
 * @file SignalSynchronizer10.cpp
 * @brief SignalSynchronizer10 实现
 *
 * 实现信号同步器：互相关峰值插值与亚采样延迟估计实现精密时间对齐。
 */

#include "utils/signal297/SignalSynchronizer10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalSynchronizer10::SignalSynchronizer10(QObject *parent)
    : QObject(parent) {}

SignalSynchronizer10::~SignalSynchronizer10() = default;

/* ---- Configuration ---- */

void SignalSynchronizer10::setSampleRate(double rate) { m_sampleRate = qBound(1.0, rate, 1e8); }

/* ---- Next power of 2 ---- */

int SignalSynchronizer10::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- Reverse bits ---- */

int SignalSynchronizer10::reverseBits(int val, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

/* ---- In-place FFT ---- */

void SignalSynchronizer10::fftInPlace(QVector<double>& real, QVector<double>& imag) const
{
    int N = real.size();
    int bits = 0;
    { int n = N; while (n > 1) { n >>= 1; bits++; } }

    // Bit reversal
    for (int i = 0; i < N; ++i) {
        int j = reverseBits(i, bits);
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    // Cooley-Tukey radix-2 DIT
    for (int size = 2; size <= N; size *= 2) {
        int halfSize = size / 2;
        double angleStep = -2.0 * M_PI / size;
        for (int i = 0; i < N; i += size) {
            for (int j = 0; j < halfSize; ++j) {
                double angle = angleStep * j;
                double wr = qCos(angle);
                double wi = qSin(angle);

                double tR = real[i + j + halfSize] * wr - imag[i + j + halfSize] * wi;
                double tI = real[i + j + halfSize] * wi + imag[i + j + halfSize] * wr;

                real[i + j + halfSize] = real[i + j] - tR;
                imag[i + j + halfSize] = imag[i + j] - tI;
                real[i + j] += tR;
                imag[i + j] += tI;
            }
        }
    }
}

/* ---- Cross-correlation via FFT ---- */

QVector<double> SignalSynchronizer10::crossCorrelation(const QVector<double>& a,
                                                          const QVector<double>& b) const
{
    int n = qMax(a.size(), b.size());
    int N = nextPow2(2 * n);

    QVector<double> realA(N, 0.0), imagA(N, 0.0);
    QVector<double> realB(N, 0.0), imagB(N, 0.0);

    for (int i = 0; i < a.size(); ++i) realA[i] = a[i];
    for (int i = 0; i < b.size(); ++i) realB[i] = b[i];

    fftInPlace(realA, imagA);
    fftInPlace(realB, imagB);

    // Cross-correlation: conj(FFT(a)) * FFT(b)
    QVector<double> realC(N), imagC(N);
    for (int i = 0; i < N; ++i) {
        realC[i] = realA[i] * realB[i] + imagA[i] * imagB[i];
        imagC[i] = -imagA[i] * realB[i] + realA[i] * imagB[i];
    }

    // Inverse FFT
    for (int i = 0; i < N; ++i) imagC[i] = -imagC[i];
    fftInPlace(realC, imagC);
    for (int i = 0; i < N; ++i) {
        realC[i] /= N;
        imagC[i] = -imagC[i] / N;
    }

    // Return correlation values (shift by N for negative lags)
    QVector<double> corr(N, 0.0);
    for (int i = 0; i < N; ++i)
        corr[i] = realC[(i + N / 2) % N];

    return corr;
}

/* ---- Parabolic interpolation for subsample peak ---- */

double SignalSynchronizer10::interpolatePeak(const QVector<double>& correlation,
                                                int peakIndex) const
{
    int n = correlation.size();
    if (n == 0 || peakIndex < 1 || peakIndex >= n - 1) return static_cast<double>(peakIndex);

    // Parabolic interpolation: delta = 0.5 * (y[k-1] - y[k+1]) / (y[k-1] - 2*y[k] + y[k+1])
    double y0 = correlation[peakIndex - 1];
    double y1 = correlation[peakIndex];
    double y2 = correlation[peakIndex + 1];

    double denom = y0 - 2.0 * y1 + y2;
    if (qAbs(denom) < 1e-15) return static_cast<double>(peakIndex);

    double delta = 0.5 * (y0 - y2) / denom;
    return static_cast<double>(peakIndex) + delta;
}

/* ---- Fractional delay interpolation ---- */

double SignalSynchronizer10::fractionalDelay(const QVector<double>& sig,
                                                double index) const
{
    int n = sig.size();
    int i0 = qFloor(index);
    int i1 = i0 + 1;
    double frac = index - i0;

    if (i0 < 0) return 0.0;
    if (i1 >= n) return (i0 < n) ? sig[i0] : 0.0;

    // Linear interpolation
    return sig[i0] * (1.0 - frac) + sig[i1] * frac;
}

/* ---- Synchronize ---- */

SignalSynchronizer10::SyncResult SignalSynchronizer10::synchronize(
    const QVector<double>& reference, const QVector<double>& delayed)
{
    QElapsedTimer timer;
    timer.start();

    SyncResult result;

    if (reference.isEmpty() || delayed.isEmpty()) {
        result.valid = false;
        return result;
    }

    // Compute cross-correlation
    auto corr = crossCorrelation(reference, delayed);
    int N = corr.size();
    int halfN = N / 2;

    // Find peak correlation
    double maxCorr = -1e300;
    int peakIdx = 0;
    for (int i = 0; i < N; ++i) {
        if (corr[i] > maxCorr) {
            maxCorr = corr[i];
            peakIdx = i;
        }
    }

    // Map to delay in samples (accounting for FFT shift)
    int rawDelay = peakIdx - halfN;

    // Parabolic interpolation for subsample precision
    double interpPeak = interpolatePeak(corr, peakIdx);
    double subsampleOffset = interpPeak - peakIdx;

    result.integerDelay = rawDelay;
    result.delaySamples = static_cast<double>(rawDelay) + subsampleOffset;
    result.delaySeconds = result.delaySamples / m_sampleRate;
    result.correlationPeak = maxCorr;

    // Compute SNR estimate (peak vs mean of correlation)
    double meanCorr = 0.0;
    for (double c : corr) meanCorr += qAbs(c);
    meanCorr /= N;
    result.snr = meanCorr > 0 ? maxCorr / meanCorr : 0.0;
    result.valid = true;

    double elapsed = timer.elapsed();
    m_stats.signalLength = reference.size();
    m_stats.totalSyncs++;
    m_corrSum += maxCorr;
    m_stats.avgCorrelation = m_corrSum / m_stats.totalSyncs;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSyncs;

    emit syncDone(result.delaySamples, maxCorr, elapsed);
    return result;
}

/* ---- Align signals ---- */

QVector<double> SignalSynchronizer10::alignSignals(const QVector<double>& delayed,
                                                      double delaySamples) const
{
    int n = delayed.size();
    int shift = qFloor(delaySamples);
    double frac = delaySamples - shift;

    QVector<double> aligned(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double srcIdx = static_cast<double>(i) - frac;
        int srcInt = static_cast<int>(srcIdx) - shift;

        if (srcInt >= 0 && srcInt < n - 1) {
            double f = srcIdx - qFloor(srcIdx);
            aligned[i] = delayed[srcInt] * (1.0 - f) + delayed[srcInt + 1] * f;
        } else if (srcInt >= 0 && srcInt < n) {
            aligned[i] = delayed[srcInt];
        }
    }
    return aligned;
}

/* ---- Reset ---- */

void SignalSynchronizer10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_corrSum = 0.0;
}
