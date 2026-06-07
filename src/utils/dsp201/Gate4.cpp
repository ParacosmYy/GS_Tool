/**
 * @file Gate4.cpp
 * @brief Gate4 实现
 *
 * 实现FFT频谱门控噪声抑制：自适应阈值估计、时频掩码平滑、音乐噪声抑制。
 */

#include "utils/dsp201/Gate4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Gate4::Gate4(QObject *parent) : QObject(parent)
{
    m_window = hannWindow(m_frameSize);
}

Gate4::~Gate4() = default;

/* ---- Configuration ---- */

void Gate4::setFrameSize(int size) { m_frameSize = qMax(64, size); m_window = hannWindow(m_frameSize); }
void Gate4::setThreshold(double thresholdDb) { m_thresholdDb = thresholdDb; }
void Gate4::setAttack(double ms) { m_attackMs = qMax(0.1, ms); }
void Gate4::setRelease(double ms) { m_releaseMs = qMax(1.0, ms); }
void Gate4::setReduction(double db) { m_reductionDb = db; }
void Gate4::setSmoothingFrames(int frames) { m_smoothingFrames = qMax(1, frames); }

/* ---- Compute Hann window ---- */

QVector<double> Gate4::hannWindow(int n) const
{
    QVector<double> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
    return w;
}

/* ---- RMS in dB ---- */

double Gate4::rmsDb(const QVector<double>& samples)
{
    double sum = 0.0;
    for (double s : samples) sum += s * s;
    double rms = qSqrt(sum / qMax(1, samples.size()));
    return 20.0 * qLn(qMax(rms, 1e-10)) / qLn(10.0);
}

/* ---- Radix-2 FFT ---- */

void Gate4::fft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { qSwap(real[i], real[j]); qSwap(imag[i], imag[j]); }
    }
    // Butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double curR = 1.0, curI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tR = curR * real[i + j + len / 2] - curI * imag[i + j + len / 2];
                double tI = curR * imag[i + j + len / 2] + curI * real[i + j + len / 2];
                real[i + j + len / 2] = real[i + j] - tR;
                imag[i + j + len / 2] = imag[i + j] - tI;
                real[i + j] += tR;
                imag[i + j] += tI;
                double newR = curR * wR - curI * wI;
                curI = curR * wI + curI * wR;
                curR = newR;
            }
        }
    }
}

/* ---- Inverse FFT ---- */

void Gate4::ifft(QVector<double>& real, QVector<double>& imag) const
{
    int n = real.size();
    for (int i = 0; i < n; ++i) imag[i] = -imag[i];
    fft(real, imag);
    for (int i = 0; i < n; ++i) { real[i] /= n; imag[i] = -imag[i] / n; }
}

/* ---- Magnitude spectrum ---- */

QVector<double> Gate4::magnitudeSpectrum(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> real(n), imag(n, 0.0);
    for (int i = 0; i < n; ++i) real[i] = frame[i] * (i < m_window.size() ? m_window[i] : 1.0);
    fft(real, imag);

    QVector<double> mag(n / 2 + 1);
    for (int i = 0; i <= n / 2; ++i)
        mag[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
    return mag;
}

/* ---- Estimate noise floor ---- */

void Gate4::estimateNoiseFloor(const QVector<QVector<double>>& noiseFrames)
{
    if (noiseFrames.isEmpty()) return;
    int bins = noiseFrames[0].size() / 2 + 1;
    m_noiseEstimate.resize(bins);
    for (int i = 0; i < bins; ++i) m_noiseEstimate[i] = 0.0;

    for (const auto& frame : noiseFrames) {
        QVector<double> mag = magnitudeSpectrum(frame);
        for (int i = 0; i < qMin(mag.size(), bins); ++i)
            m_noiseEstimate[i] += mag[i];
    }
    for (int i = 0; i < bins; ++i)
        m_noiseEstimate[i] /= noiseFrames.size();

    m_stats.noiseFloorDb = rmsDb(m_noiseEstimate);
}

/* ---- Apply spectral gate ---- */

QVector<double> Gate4::applySpectralGate(const QVector<double>& spectrum,
                                           const QVector<double>& noiseEst) const
{
    int n = spectrum.size();
    QVector<double> mask(n, 0.0);
    double threshLinear = qPow(10.0, m_thresholdDb / 20.0);

    for (int i = 0; i < n; ++i) {
        double noiseRef = (i < noiseEst.size()) ? noiseEst[i] : 0.0;
        double threshold = qMax(noiseRef * threshLinear, 1e-10);
        // Soft gate: ratio-based suppression
        if (spectrum[i] > threshold) {
            mask[i] = 1.0;
        } else {
            double ratio = spectrum[i] / threshold;
            mask[i] = ratio * ratio;  // Smooth transition
        }
    }
    return mask;
}

/* ---- Smooth mask to suppress musical noise ---- */

QVector<double> Gate4::smoothMask(const QVector<double>& mask) const
{
    QVector<double> smoothed = mask;
    for (int iter = 0; iter < m_smoothingFrames; ++iter) {
        QVector<double> prev = smoothed;
        for (int i = 1; i < smoothed.size() - 1; ++i)
            smoothed[i] = 0.25 * prev[i - 1] + 0.5 * prev[i] + 0.25 * prev[i + 1];
    }
    // Temporal smoothing with previous mask
    if (m_prevMask.size() == smoothed.size()) {
        double alpha = 0.7;
        for (int i = 0; i < smoothed.size(); ++i)
            smoothed[i] = alpha * smoothed[i] + (1.0 - alpha) * m_prevMask[i];
    }
    m_prevMask = smoothed;
    return smoothed;
}

/* ---- Process frame ---- */

QVector<double> Gate4::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    int n = frame.size();
    if (n != m_frameSize) { m_frameSize = n; m_window = hannWindow(n); }

    double rmsIn = rmsDb(frame);

    // FFT
    QVector<double> real(n), imag(n, 0.0);
    for (int i = 0; i < n; ++i) real[i] = frame[i] * m_window[i];
    fft(real, imag);

    // Compute magnitude
    QVector<double> mag(n / 2 + 1);
    for (int i = 0; i <= n / 2; ++i)
        mag[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);

    // Spectral gating
    QVector<double> mask = applySpectralGate(mag, m_noiseEstimate);
    mask = smoothMask(mask);

    // Apply mask
    for (int i = 0; i <= n / 2; ++i) {
        real[i] *= mask[i];
        imag[i] *= mask[i];
    }
    // Mirror for negative frequencies
    for (int i = n / 2 + 1; i < n; ++i) {
        real[i] = real[n - i] * mask[n - i];
        imag[i] = -imag[n - i] * mask[n - i];
    }

    // IFFT
    ifft(real, imag);

    QVector<double> output(n);
    for (int i = 0; i < n; ++i) output[i] = real[i];

    double rmsOut = rmsDb(output);

    m_stats.totalFrames++;
    m_stats.frameSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(rmsIn, rmsOut, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Gate4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_prevMask.clear();
}
