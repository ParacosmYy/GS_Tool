/**
 * @file SpectralGate8.cpp
 * @brief SpectralGate8 实现
 *
 * 实现频谱门：Wiener滤波器与噪声估计跟踪和频谱下限的音乐噪声伪影抑制。
 */

#include "utils/dsp279/SpectralGate8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralGate8::SpectralGate8(QObject *parent)
    : QObject(parent)
{
    m_noiseEstimate.resize(m_frameSize, 0.0);
    m_prevMagnitude.resize(m_frameSize, 0.0);
    m_prevGain.resize(m_frameSize, 1.0);
}

SpectralGate8::~SpectralGate8() = default;

/* ---- Configuration ---- */

void SpectralGate8::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
    m_noiseEstimate.resize(m_frameSize, 0.0);
    m_prevMagnitude.resize(m_frameSize, 0.0);
    m_prevGain.resize(m_frameSize, 1.0);
}

void SpectralGate8::setSpectralFloor(double floor) { m_spectralFloor = qBound(0.0, floor, 1.0); }
void SpectralGate8::setNoiseEstimationRate(double rate) { m_noiseRate = qBound(0.5, rate, 0.999); }
void SpectralGate8::setOversubtractionFactor(double factor) { m_oversubtraction = qBound(0.5, factor, 3.0); }

/* ---- Apply Hann window ---- */

QVector<double> SpectralGate8::applyWindow(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> windowed(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / n));
        windowed[i] = frame[i] * w;
    }
    return windowed;
}

/* ---- Radix-2 FFT ---- */

void SpectralGate8::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    if (n <= 1) return;

    // Bit-reversal permutation
    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) ++bits;
    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        }
        if (rev > i) {
            std::swap(real[i], real[rev]);
            std::swap(imag[i], imag[rev]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uR = real[i + j], uI = imag[i + j];
                double tR = cR * real[i + j + len / 2] - cI * imag[i + j + len / 2];
                double tI = cR * imag[i + j + len / 2] + cI * real[i + j + len / 2];
                real[i + j] = uR + tR;
                imag[i + j] = uI + tI;
                real[i + j + len / 2] = uR - tR;
                imag[i + j + len / 2] = uI - tI;
                double newC = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = newC;
            }
        }
    }
}

/* ---- Inverse FFT ---- */

void SpectralGate8::ifft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
}

/* ---- Update noise estimate via minimum statistics ---- */

void SpectralGate8::updateNoiseEstimate(const QVector<double>& magnitude)
{
    int n = magnitude.size();
    for (int i = 0; i < n; ++i) {
        // Recursive smoothing for noise floor tracking
        if (magnitude[i] < m_noiseEstimate[i]) {
            // Fast adaptation downward
            m_noiseEstimate[i] = 0.9 * magnitude[i] + 0.1 * m_noiseEstimate[i];
        } else {
            // Slow adaptation upward
            m_noiseEstimate[i] = m_noiseRate * m_noiseEstimate[i] +
                                  (1.0 - m_noiseRate) * magnitude[i];
        }
    }
    m_frameCount++;
    double noisePower = 0.0;
    for (int i = 0; i < n; ++i) noisePower += m_noiseEstimate[i];
    emit noiseEstimateUpdated(noisePower / n);
}

/* ---- Compute Wiener gain with spectral floor ---- */

QVector<double> SpectralGate8::computeWienerGain(const QVector<double>& magnitude) const
{
    int n = magnitude.size();
    QVector<double> gain(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double noisePow = qMax(m_noiseEstimate[i] * m_oversubtraction, 1e-10);
        double snr = (magnitude[i] - noisePow) / qMax(noisePow, 1e-10);
        // Wiener gain: max(snr / (1 + snr), spectralFloor)
        double wGain = qMax(snr / (1.0 + qMax(snr, 0.0)), m_spectralFloor);
        gain[i] = qBound(m_spectralFloor, wGain, 1.0);
    }
    return gain;
}

/* ---- Compute musical noise index ---- */

double SpectralGate8::computeMusicalNoiseIndex(const QVector<double>& gain) const
{
    // Measure sparsity of gain as proxy for musical noise
    int n = gain.size();
    double mean = 0.0;
    for (int i = 0; i < n; ++i) mean += gain[i];
    mean /= n;
    double variance = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = gain[i] - mean;
        variance += d * d;
    }
    return variance / n;   // Higher variance = more musical noise artifacts
}

/* ---- Process one frame ---- */

SpectralGate8::GateResult SpectralGate8::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    GateResult result;
    int n = qMin(frame.size(), m_frameSize);
    if (n < 4) return result;

    // Window and copy
    QVector<double> windowed = applyWindow(frame);
    QVector<double> real(n, 0.0), imag(n, 0.0);
    for (int i = 0; i < n; ++i) real[i] = windowed[i];

    // FFT
    fft(real, imag);

    // Compute magnitude spectrum
    QVector<double> magnitude(n, 0.0);
    for (int i = 0; i < n; ++i)
        magnitude[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);

    // Update noise estimate
    if (m_frameCount < 5) {
        // First few frames: assume noise-only
        for (int i = 0; i < n; ++i) m_noiseEstimate[i] = magnitude[i];
    } else {
        updateNoiseEstimate(magnitude);
    }

    // Compute Wiener gain
    QVector<double> gain = computeWienerGain(magnitude);

    // Apply gain to spectrum
    for (int i = 0; i < n; ++i) {
        real[i] *= gain[i];
        imag[i] *= gain[i];
    }

    // Inverse FFT
    ifft(real, imag);

    // Extract clean signal
    result.cleanSignal.resize(n);
    for (int i = 0; i < n; ++i) result.cleanSignal[i] = real[i];

    result.spectrum = magnitude;
    result.noiseEstimate = m_noiseEstimate;
    result.gainCurve = gain;
    result.musicalNoiseIndex = computeMusicalNoiseIndex(gain);

    // Compute SNR
    double sigPow = 0.0, noisePow = 0.0;
    for (int i = 0; i < n; ++i) {
        sigPow += magnitude[i] * magnitude[i];
        noisePow += m_noiseEstimate[i] * m_noiseEstimate[i];
    }
    result.snr = (noisePow > 1e-10) ? 10.0 * qLn(sigPow / noisePow) / qLn(10.0) : 0.0;

    double elapsed = timer.elapsed();
    m_stats.frameSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    m_snrSum += result.snr;
    m_stats.avgSnr = m_snrSum / m_stats.totalOps;
    emit frameProcessed(m_frameCount, result.snr, elapsed);

    return result;
}

/* ---- Process entire signal ---- */

QVector<SpectralGate8::GateResult> SpectralGate8::processSignal(const QVector<double>& signal)
{
    QVector<GateResult> results;
    int hop = m_frameSize / 2;  // 50% overlap
    for (int i = 0; i + m_frameSize <= signal.size(); i += hop) {
        QVector<double> frame(m_frameSize);
        for (int j = 0; j < m_frameSize; ++j) frame[j] = signal[i + j];
        results.append(process(frame));
    }
    return results;
}

/* ---- Reset noise estimate ---- */

void SpectralGate8::resetNoiseEstimate()
{
    m_noiseEstimate.fill(0.0);
    m_prevMagnitude.fill(0.0);
    m_prevGain.fill(1.0);
    m_frameCount = 0;
}

/* ---- Reset ---- */

void SpectralGate8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_snrSum = 0.0;
    resetNoiseEstimate();
}
