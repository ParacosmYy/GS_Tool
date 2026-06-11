/**
 * @file EnvelopeDetector10.cpp
 * @brief EnvelopeDetector10 实现
 *
 * 实现包络检测器：解析信号分解与频率加权Hilbert变换瞬时幅度估计。
 */

#include "utils/signal289/EnvelopeDetector10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector10::EnvelopeDetector10(QObject *parent)
    : QObject(parent)
{
    setConfig(DetectorConfig{});
}

EnvelopeDetector10::~EnvelopeDetector10() = default;

/* ---- Configuration ---- */

void EnvelopeDetector10::setConfig(const DetectorConfig& cfg)
{
    m_config = cfg;
    m_config.sampleRate = qBound(8000.0, m_config.sampleRate, 192000.0);
    m_config.fftSize = qBound(64, m_config.fftSize, 65536);
    designHilbertFilter();
    computeFreqWeights();
}

/* ---- Design FIR Hilbert filter via frequency sampling ---- */

void EnvelopeDetector10::designHilbertFilter()
{
    // Hilbert transformer: H(f) = -j*sgn(f) for |f| in (0, 0.5)
    // Implemented via spectral domain in analyticSignal()
    int N = m_config.fftSize;
    m_hilbertCoeffs.resize(N);
    for (int i = 0; i < N; ++i) m_hilbertCoeffs[i] = 0.0;
}

/* ---- Compute frequency-dependent weights ---- */

void EnvelopeDetector10::computeFreqWeights()
{
    int N = m_config.fftSize;
    m_freqWeights.resize(N / 2 + 1);
    double fc = (m_config.freqCutoff > 0) ? m_config.freqCutoff / m_config.sampleRate : 0.1;
    for (int i = 0; i <= N / 2; ++i) {
        double f = static_cast<double>(i) / N;
        // Linear interpolation between low and high frequency weights
        if (f < fc)
            m_freqWeights[i] = m_config.lowFreqWeight;
        else
            m_freqWeights[i] = m_config.lowFreqWeight +
                (m_config.highFreqWeight - m_config.lowFreqWeight) * (f - fc) / (0.5 - fc);
    }
}

/* ---- Next power of 2 ---- */

int EnvelopeDetector10::nextPow2(int n) { int p = 1; while (p < n) p <<= 1; return p; }

/* ---- In-place FFT ---- */

void EnvelopeDetector10::fft(QVector<double>& re, QVector<double>& im, int n, bool inverse)
{
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    double dir = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= n; len <<= 1) {
        double angle = dir * 2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe; im[v] = im[u] - tIm;
                re[u] = re[u] + tRe; im[u] = im[u] + tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
    if (inverse) {
        for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
    }
}

/* ---- Apply frequency-weighted Hilbert transform in spectral domain ---- */

void EnvelopeDetector10::applyWeightedHilbert(QVector<double>& re, QVector<double>& im, int n)
{
    // Zero negative frequencies, apply weights to positive
    for (int i = 0; i <= n / 2; ++i) {
        double w = (i < m_freqWeights.size()) ? m_freqWeights[i] : 1.0;
        re[i] *= w;
        im[i] *= w;
    }
    for (int i = n / 2 + 1; i < n; ++i) {
        re[i] = 0.0;
        im[i] = 0.0;
    }
    // Double positive frequencies (DC and Nyquist remain)
    for (int i = 1; i < n / 2; ++i) {
        re[i] *= 2.0;
        im[i] *= 2.0;
    }
}

/* ---- Analytic signal ---- */

void EnvelopeDetector10::analyticSignal(const QVector<double>& input,
                                         QVector<double>& realOut,
                                         QVector<double>& imagOut)
{
    int n = input.size();
    int N = nextPow2(n);

    QVector<double> re(N, 0.0), im(N, 0.0);
    for (int i = 0; i < n; ++i) re[i] = input[i];

    fft(re, im, N, false);
    applyWeightedHilbert(re, im, N);
    fft(re, im, N, true);

    realOut.resize(n);
    imagOut.resize(n);
    for (int i = 0; i < n; ++i) {
        realOut[i] = re[i];
        imagOut[i] = im[i];
    }
}

/* ---- Detect (full result) ---- */

EnvelopeDetector10::EnvelopeResult EnvelopeDetector10::detect(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    EnvelopeResult result;
    int n = input.size();
    if (n == 0) return result;

    // Compute analytic signal
    QVector<double> realPart, imagPart;
    analyticSignal(input, realPart, imagPart);

    // Compute instantaneous amplitude (envelope)
    result.envelope.resize(n);
    result.instPhase.resize(n);
    result.instFreq.resize(n);

    double peakEnv = 0.0;
    double sumSq = 0.0;

    for (int i = 0; i < n; ++i) {
        double env = qSqrt(realPart[i] * realPart[i] + imagPart[i] * imagPart[i]);
        result.envelope[i] = env;
        result.instPhase[i] = qAtan2(imagPart[i], realPart[i]);

        if (env > peakEnv) peakEnv = env;
        sumSq += env * env;

        // Instantaneous frequency from phase derivative
        if (i > 0) {
            double dPhase = result.instPhase[i] - result.instPhase[i - 1];
            // Unwrap phase
            while (dPhase > M_PI) dPhase -= 2.0 * M_PI;
            while (dPhase < -M_PI) dPhase += 2.0 * M_PI;
            result.instFreq[i] = dPhase * m_config.sampleRate / (2.0 * M_PI);
        } else {
            result.instFreq[i] = 0.0;
        }
    }

    result.peakEnvelope = peakEnv;
    result.rmsEnvelope = qSqrt(sumSq / n);

    double elapsed = timer.elapsed();
    m_stats.totalSamples += n;
    m_stats.numFrames++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit detectionDone(n, peakEnv, elapsed);
    return result;
}

/* ---- Simple envelope (no freq weighting) ---- */

QVector<double> EnvelopeDetector10::detectSimple(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> envelope(n);
    for (int i = 0; i < n; ++i) envelope[i] = qAbs(input[i]);

    // Smooth with simple one-pole filter
    double alpha = 0.95;
    for (int i = 1; i < n; ++i)
        envelope[i] = alpha * envelope[i - 1] + (1.0 - alpha) * envelope[i];
    return envelope;
}

/* ---- Reset ---- */

void EnvelopeDetector10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
