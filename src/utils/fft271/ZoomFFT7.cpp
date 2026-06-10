/**
 * @file ZoomFFT7.cpp
 * @brief ZoomFFT7 实现
 *
 * 实现缩放FFT：复数频移抽取与窄带谱分析高分辨率频率缩放。
 */

#include "utils/fft271/ZoomFFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT7::ZoomFFT7(QObject *parent)
    : QObject(parent) {}

ZoomFFT7::~ZoomFFT7() = default;

/* ---- Configuration ---- */

void ZoomFFT7::setCenterFrequency(double fc)
{
    m_centerFreq = qBound(0.0, fc, 0.5);
}

void ZoomFFT7::setDecimationFactor(int d)
{
    m_decimation = qBound(2, d, 64);
}

void ZoomFFT7::setFFTSize(int n)
{
    // Must be power of 2
    int p = 1;
    while (p < n) p <<= 1;
    m_fftSize = qBound(64, p, 65536);
}

/* ---- Twiddle factor ---- */

ZoomFFT7::Complex ZoomFFT7::twiddle(int k, int n, bool inverse) const
{
    double angle = -2.0 * M_PI * k / n;
    if (inverse) angle = -angle;
    return {qCos(angle), qSin(angle)};
}

/* ---- Frequency shift to baseband ---- */

QVector<ZoomFFT7::Complex> ZoomFFT7::frequencyShift(const QVector<double>& input) const
{
    int n = input.size();
    QVector<Complex> shifted(n);

    for (int i = 0; i < n; ++i) {
        double angle = -2.0 * M_PI * m_centerFreq * i;
        Complex mixer{qCos(angle), qSin(angle)};
        shifted[i].re = input[i] * mixer.re;
        shifted[i].im = input[i] * mixer.im;
    }
    return shifted;
}

/* ---- Low-pass filter and decimate ---- */

QVector<ZoomFFT7::Complex> ZoomFFT7::decimate(const QVector<Complex>& shifted) const
{
    int n = shifted.size();
    int halfBand = m_decimation / 2;

    // Simple moving-average low-pass filter then decimate
    // Filter length = 2 * halfBand + 1
    int filtLen = 2 * halfBand + 1;
    double norm = 1.0 / filtLen;

    QVector<Complex> decimated;
    decimated.reserve(n / m_decimation + 1);

    for (int i = halfBand; i < n - halfBand; i += m_decimation) {
        Complex acc{0.0, 0.0};
        for (int j = -halfBand; j <= halfBand; ++j) {
            acc.re += shifted[i + j].re * norm;
            acc.im += shifted[i + j].im * norm;
        }
        decimated.append(acc);
    }
    return decimated;
}

/* ---- Cooley-Tukey radix-2 FFT ---- */

QVector<ZoomFFT7::Complex> ZoomFFT7::fft(const QVector<Complex>& data) const
{
    int n = data.size();
    if (n <= 1) return data;

    // Pad or truncate to m_fftSize
    int N = m_fftSize;
    QVector<Complex> x(N);
    for (int i = 0; i < N; ++i) {
        if (i < n)
            x[i] = data[i];
        else
            x[i] = {0.0, 0.0};
    }

    // Bit-reversal permutation
    int bits = 0;
    while ((1 << bits) < N) ++bits;
    for (int i = 0; i < N; ++i) {
        int rev = 0;
        int val = i;
        for (int b = 0; b < bits; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        if (rev > i) {
            Complex tmp = x[i];
            x[i] = x[rev];
            x[rev] = tmp;
        }
    }

    // Butterfly stages
    for (int len = 2; len <= N; len <<= 1) {
        int halfLen = len >> 1;
        for (int i = 0; i < N; i += len) {
            for (int j = 0; j < halfLen; ++j) {
                Complex w = twiddle(j, len, false);
                Complex t{w.re * x[i + j + halfLen].re - w.im * x[i + j + halfLen].im,
                          w.re * x[i + j + halfLen].im + w.im * x[i + j + halfLen].re};
                x[i + j + halfLen].re = x[i + j].re - t.re;
                x[i + j + halfLen].im = x[i + j].im - t.im;
                x[i + j].re += t.re;
                x[i + j].im += t.im;
            }
        }
    }

    return x;
}

/* ---- Main processing pipeline ---- */

QVector<ZoomFFT7::Complex> ZoomFFT7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    // Step 1: Frequency shift to baseband
    QVector<Complex> shifted = frequencyShift(input);

    // Step 2: Low-pass filter and decimate
    QVector<Complex> decimated = decimate(shifted);

    // Step 3: FFT on decimated data
    m_spectrum = fft(decimated);

    // Step 4: Compute magnitude
    int specN = m_spectrum.size();
    m_magnitude.resize(specN);
    for (int i = 0; i < specN; ++i) {
        double mag = qSqrt(m_spectrum[i].re * m_spectrum[i].re
                           + m_spectrum[i].im * m_spectrum[i].im);
        m_magnitude[i] = 20.0 * qLn(qMax(mag, 1e-10)) / qLn(10.0);
    }

    double elapsed = timer.elapsed();
    m_stats.inputSize = n;
    m_stats.fftSize = m_fftSize;
    m_stats.decimationFactor = m_decimation;
    m_stats.centerFreq = m_centerFreq;
    m_stats.bandwidth = 1.0 / m_decimation;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit spectrumReady(m_fftSize, m_centerFreq, elapsed);

    return m_spectrum;
}

/* ---- Magnitude spectrum ---- */

QVector<double> ZoomFFT7::magnitudeSpectrum() const
{
    return m_magnitude;
}

/* ---- Frequency axis ---- */

QVector<double> ZoomFFT7::frequencyAxis(double sampleRate) const
{
    int n = m_fftSize;
    QVector<double> freqs(n);
    double bw = sampleRate / m_decimation;
    double fStart = (m_centerFreq - 0.5 / m_decimation) * sampleRate;

    for (int i = 0; i < n; ++i)
        freqs[i] = fStart + static_cast<double>(i) * bw / n;

    return freqs;
}

/* ---- Reset ---- */

void ZoomFFT7::resetStatistics()
{
    m_spectrum.clear();
    m_magnitude.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
