/**
 * @file ZoomFFT9.cpp
 * @brief ZoomFFT9 实现
 *
 * 实现缩放FFT：多相抽取滤波与复数带通窄带高分辨率频谱缩放。
 */

#include "utils/fft288/ZoomFFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT9::ZoomFFT9(QObject *parent)
    : QObject(parent)
{
    setConfig(ZoomConfig{});
}

ZoomFFT9::~ZoomFFT9() = default;

/* ---- Configuration ---- */

void ZoomFFT9::setConfig(const ZoomConfig& cfg)
{
    m_config = cfg;
    m_config.decimationFactor = qBound(2, m_config.decimationFactor, 256);
    m_config.fftSize = qBound(64, m_config.fftSize, 65536);
    m_config.filterTaps = qBound(8, m_config.filterTaps, 512);
    m_config.centerFreq = qBound(0.0, m_config.centerFreq, 0.5);
    m_config.bandwidth = qBound(0.001, m_config.bandwidth, 0.5);
    designFilter();
}

void ZoomFFT9::setSampleRate(double rate) { m_sampleRate = qBound(1.0, rate, 1e9); }

/* ---- Design prototype low-pass filter ---- */

void ZoomFFT9::designFilter()
{
    int D = m_config.decimationFactor;
    int M = m_config.filterTaps;
    double cutoff = m_config.bandwidth / (2.0 * D);

    // Design prototype FIR lowpass using windowed sinc
    int totalTaps = D * M;
    m_filterCoeffs.resize(totalTaps);
    for (int n = 0; n < totalTaps; ++n) {
        double x = static_cast<double>(n) - (totalTaps - 1) / 2.0;
        double w = blackman(n, totalTaps);
        m_filterCoeffs[n] = 2.0 * cutoff * sinc(2.0 * cutoff * x) * w;
    }
}

/* ---- Helper functions ---- */

double ZoomFFT9::sinc(double x) { return (qAbs(x) < 1e-10) ? 1.0 : qSin(M_PI * x) / (M_PI * x); }

double ZoomFFT9::blackman(int i, int N)
{
    if (N <= 1) return 1.0;
    double n = static_cast<double>(i);
    double Nm1 = static_cast<double>(N - 1);
    return 0.42 - 0.5 * qCos(2.0 * M_PI * n / Nm1) + 0.08 * qCos(4.0 * M_PI * n / Nm1);
}

int ZoomFFT9::nextPow2(int n) { int p = 1; while (p < n) p <<= 1; return p; }

/* ---- Complex mixer ---- */

QVector<double> ZoomFFT9::complexMix(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> mixed(n * 2);  // Interleaved [re0, im0, re1, im1, ...]
    double fc = m_config.centerFreq;

    for (int i = 0; i < n; ++i) {
        double phase = -2.0 * M_PI * fc * i;
        mixed[i * 2] = input[i] * qCos(phase);     // Real part
        mixed[i * 2 + 1] = input[i] * qSin(phase); // Imaginary part
    }
    return mixed;
}

/* ---- Polyphase decimation ---- */

QVector<double> ZoomFFT9::polyphaseDecimate(const QVector<double>& input) const
{
    int D = m_config.decimationFactor;
    int M = m_config.filterTaps;
    int n = input.size() / 2; // Complex samples
    int totalTaps = m_filterCoeffs.size();

    QVector<double> decRe, decIm;
    int outputCount = (n - totalTaps / D) / D + 1;
    decRe.reserve(outputCount);
    decIm.reserve(outputCount);

    for (int k = 0; k * D + totalTaps - 1 < n; ++k) {
        double reSum = 0.0, imSum = 0.0;
        for (int m = 0; m < M; ++m) {
            for (int d = 0; d < D; ++d) {
                int tapIdx = d + m * D;
                int sampIdx = k * D + m * D + d;
                if (tapIdx < totalTaps && sampIdx < n) {
                    double coeff = m_filterCoeffs[tapIdx];
                    reSum += coeff * input[sampIdx * 2];
                    imSum += coeff * input[sampIdx * 2 + 1];
                }
            }
        }
        decRe.append(reSum * D); // Compensate for decimation gain
        decIm.append(imSum * D);
    }

    // Interleave output
    QVector<double> result(decRe.size() * 2);
    for (int i = 0; i < decRe.size(); ++i) {
        result[i * 2] = decRe[i];
        result[i * 2 + 1] = decIm[i];
    }
    return result;
}

/* ---- Bit reversal ---- */

void ZoomFFT9::bitReverse(QVector<double>& re, QVector<double>& im, int n)
{
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
}

/* ---- In-place complex FFT (Cooley-Tukey) ---- */

void ZoomFFT9::computeFFT(QVector<double>& real, QVector<double>& imag, int n)
{
    bitReverse(real, imag, n);

    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle);
        double wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j;
                int v = i + j + len / 2;
                double tRe = curRe * real[v] - curIm * imag[v];
                double tIm = curRe * imag[v] + curIm * real[v];
                real[v] = real[u] - tRe;
                imag[v] = imag[u] - tIm;
                real[u] = real[u] + tRe;
                imag[u] = imag[u] + tIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }
}

/* ---- Main process ---- */

ZoomFFT9::ZoomResult ZoomFFT9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ZoomResult result;

    // Step 1: Complex frequency shift (mix to baseband)
    QVector<double> mixed = complexMix(input);

    // Step 2: Polyphase decimation
    QVector<double> decimated = polyphaseDecimate(mixed);
    int decSamples = decimated.size() / 2;

    // Step 3: Zero-pad to FFT size
    int fftN = nextPow2(qMax(decSamples, m_config.fftSize));
    QVector<double> re(fftN, 0.0), im(fftN, 0.0);
    for (int i = 0; i < decSamples; ++i) {
        re[i] = decimated[i * 2];
        im[i] = decimated[i * 2 + 1];
    }

    // Step 4: FFT
    computeFFT(re, im, fftN);

    // Step 5: Compute magnitude and phase
    int halfN = fftN / 2;
    result.magnitude.resize(halfN);
    result.phase.resize(halfN);
    result.frequencies.resize(halfN);

    double D = m_config.decimationFactor;
    double effectiveRate = m_sampleRate / D;
    double freqRes = effectiveRate / fftN;
    double fc = m_config.centerFreq * m_sampleRate;

    for (int i = 0; i < halfN; ++i) {
        result.magnitude[i] = 2.0 * qSqrt(re[i] * re[i] + im[i] * im[i]) / fftN;
        result.phase[i] = qAtan2(im[i], re[i]);
        result.frequencies[i] = fc + (i - halfN / 2.0) * freqRes;
    }
    result.effectiveBandwidth = effectiveRate / 2.0;
    result.frequencyResolution = freqRes;

    double elapsed = timer.elapsed();
    m_stats.lastFFTSize = fftN;
    m_stats.lastDecimation = m_config.decimationFactor;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit zoomDone(fftN, m_config.decimationFactor, elapsed);
    return result;
}

/* ---- Reset ---- */

void ZoomFFT9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
