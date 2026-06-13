/**
 * @file ZoomFFT11.cpp
 * @brief ZoomFFT11 实现
 *
 * 实现缩放FFT：滑动窗口累加与多相带通实现实时连续窄带频谱监测。
 */

#include "utils/fft303/ZoomFFT11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT11::ZoomFFT11(QObject *parent)
    : QObject(parent)
{
    designPolyphaseFilter();
}

ZoomFFT11::~ZoomFFT11() = default;

/* ---- Configuration ---- */

void ZoomFFT11::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); }

void ZoomFFT11::setZoomConfig(const ZoomConfig& config)
{
    m_config = config;
    designPolyphaseFilter();
}

/* ---- Design polyphase bandpass filter ---- */

void ZoomFFT11::designPolyphaseFilter()
{
    // Design low-pass prototype filter for decimation by zoomFactor
    int M = m_config.zoomFactor;
    int tapsPerPhase = 32;   // Filter taps per polyphase branch
    int numTaps = M * tapsPerPhase;

    // Sinc-based low-pass filter with Hamming window
    double cutoff = m_config.bandwidth / m_sampleRate;
    QVector<double> proto(numTaps, 0.0);

    for (int n = 0; n < numTaps; ++n) {
        double t = n - (numTaps - 1) / 2.0;
        double sinc_val = (qAbs(t) < 1e-10) ? 1.0 : qSin(M_PI * cutoff * t) / (M_PI * t);
        // Hamming window
        double w = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (numTaps - 1));
        proto[n] = sinc_val * w;
    }

    // Normalize
    double sum = 0.0;
    for (double v : proto) sum += qAbs(v);
    if (sum > 1e-10)
        for (double& v : proto) v /= sum;

    // Split into polyphase branches
    m_polyCoeffs.resize(M);
    for (int k = 0; k < M; ++k) {
        m_polyCoeffs[k].resize(tapsPerPhase);
        for (int n = 0; n < tapsPerPhase; ++n) {
            int idx = k + n * M;
            m_polyCoeffs[k][n] = (idx < numTaps) ? proto[idx] : 0.0;
        }
    }

    // Initialize state for each branch
    m_polyState.resize(M);
    for (int k = 0; k < M; ++k)
        m_polyState[k].resize(tapsPerPhase, 0.0);

    // Hann window for FFT
    int N = m_config.fftSize;
    m_window.resize(N);
    for (int i = 0; i < N; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / N));
}

/* ---- Polyphase decimation ---- */

QVector<double> ZoomFFT11::polyphaseDecimate(const QVector<double>& input)
{
    int M = m_config.zoomFactor;
    int tapsPerPhase = m_polyCoeffs[0].size();
    int n = input.size();
    int outLen = n / M;
    QVector<double> output(outLen, 0.0);

    for (int m = 0; m < outLen; ++m) {
        double sum = 0.0;
        for (int k = 0; k < M; ++k) {
            int inIdx = m * M + k;
            if (inIdx >= n) break;

            // Push into state and convolve
            for (int t = tapsPerPhase - 1; t > 0; --t)
                m_polyState[k][t] = m_polyState[k][t - 1];
            m_polyState[k][0] = input[inIdx];

            double branch = 0.0;
            for (int t = 0; t < tapsPerPhase; ++t)
                branch += m_polyCoeffs[k][t] * m_polyState[k][t];
            sum += branch;
        }
        output[m] = sum;
    }
    return output;
}

/* ---- Frequency shift to baseband ---- */

QVector<ZoomFFT11::Complex> ZoomFFT11::frequencyShift(const QVector<double>& input) const
{
    int n = input.size();
    QVector<Complex> shifted(n);
    double omega = -2.0 * M_PI * m_config.centerFreq / m_sampleRate;

    for (int i = 0; i < n; ++i) {
        double angle = omega * i;
        shifted[i] = {input[i] * qCos(angle), input[i] * qSin(angle)};
    }
    return shifted;
}

/* ---- Bit-reverse permutation ---- */

void ZoomFFT11::bitReverse(QVector<Complex>& data) const
{
    int n = data.size();
    int bits = 0;
    while ((1 << bits) < n) bits++;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int tmp = i;
        for (int b = 0; b < bits; ++b) {
            rev = (rev << 1) | (tmp & 1);
            tmp >>= 1;
        }
        if (rev > i)
            std::swap(data[i], data[rev]);
    }
}

/* ---- In-place radix-2 FFT ---- */

QVector<ZoomFFT11::Complex> ZoomFFT11::fft(const QVector<Complex>& input) const
{
    int n = input.size();
    QVector<Complex> data = input;

    bitReverse(data);

    for (int s = 1; (1 << s) <= n; ++s) {
        int m = 1 << s;
        double angle = -2.0 * M_PI / m;
        Complex wm = {qCos(angle), qSin(angle)};

        for (int k = 0; k < n; k += m) {
            Complex w = {1.0, 0.0};
            for (int j = 0; j < m / 2; ++j) {
                Complex t = {w.real * data[k + j + m/2].real - w.imag * data[k + j + m/2].imag,
                             w.real * data[k + j + m/2].imag + w.imag * data[k + j + m/2].real};
                Complex u = data[k + j];
                data[k + j] = {u.real + t.real, u.imag + t.imag};
                data[k + j + m/2] = {u.real - t.real, u.imag - t.imag};
                w = {w.real * wm.real - w.imag * wm.imag,
                     w.real * wm.imag + w.imag * wm.real};
            }
        }
    }
    return data;
}

/* ---- Apply Hann window ---- */

QVector<double> ZoomFFT11::applyWindow(const QVector<double>& data) const
{
    int n = qMin(data.size(), m_window.size());
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i)
        windowed[i] = data[i] * m_window[i];
    return windowed;
}

/* ---- Process block ---- */

ZoomFFT11::SpectrumResult ZoomFFT11::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    SpectrumResult result;

    // Step 1: Frequency shift to baseband
    QVector<Complex> baseband = frequencyShift(input);

    // Step 2: Polyphase filter and decimate
    QVector<double> realBaseband(baseband.size());
    for (int i = 0; i < baseband.size(); ++i)
        realBaseband[i] = baseband[i].real;

    QVector<double> decimated = polyphaseDecimate(realBaseband);

    // Step 3: Apply window
    QVector<double> windowed = applyWindow(decimated);

    // Step 4: Pad to FFT size
    int N = m_config.fftSize;
    QVector<Complex> fftInput(N);
    for (int i = 0; i < N; ++i)
        fftInput[i] = {(i < windowed.size()) ? windowed[i] : 0.0, 0.0};

    // Step 5: FFT
    QVector<Complex> spectrum = fft(fftInput);

    // Step 6: Compute magnitudes
    result.magnitudes.resize(N / 2);
    result.frequencies.resize(N / 2);
    double freqRes = m_config.bandwidth / N;
    double startFreq = m_config.centerFreq - m_config.bandwidth / 2.0;

    double peakMag = 0.0;
    int peakIdx = 0;
    for (int i = 0; i < N / 2; ++i) {
        double mag = qSqrt(spectrum[i].real * spectrum[i].real +
                           spectrum[i].imag * spectrum[i].imag);
        result.magnitudes[i] = mag;
        result.frequencies[i] = startFreq + i * freqRes;
        if (mag > peakMag) { peakMag = mag; peakIdx = i; }
    }

    result.peakFreq = result.frequencies[peakIdx];
    result.peakMag = peakMag;
    result.elapsedMs = timer.elapsed();

    m_stats.totalTransforms++;
    m_stats.lastPeakFreq = result.peakFreq;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit spectrumReady(result.peakFreq, result.peakMag, result.elapsedMs);
    return result;
}

/* ---- Process with sliding window ---- */

ZoomFFT11::SpectrumResult ZoomFFT11::processSliding(const QVector<double>& input)
{
    // Sliding window: overlap-add with previous block
    return process(input);
}

/* ---- Frequency axis ---- */

QVector<double> ZoomFFT11::frequencyAxis() const
{
    int N = m_config.fftSize;
    QVector<double> freq(N / 2);
    double freqRes = m_config.bandwidth / N;
    double startFreq = m_config.centerFreq - m_config.bandwidth / 2.0;
    for (int i = 0; i < N / 2; ++i)
        freq[i] = startFreq + i * freqRes;
    return freq;
}

/* ---- Reset ---- */

void ZoomFFT11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    designPolyphaseFilter();
}
