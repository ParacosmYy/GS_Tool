/**
 * @file EnvelopeDetector2.cpp
 * @brief EnvelopeDetector2 实现
 *
 * 实现包络检测：Hilbert变换解析信号法、峰值追踪、瞬时相位/频率。
 */

#include "utils/signal172/EnvelopeDetector2.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction ---- */

EnvelopeDetector2::EnvelopeDetector2(QObject *parent)
    : QObject(parent)
{
    double frameDur = 1.0 / m_sampleRate;
    m_attackCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_attackMs / 1000.0));
    m_releaseCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_releaseMs / 1000.0));
}

EnvelopeDetector2::~EnvelopeDetector2() = default;

/* ---- Configuration ---- */

void EnvelopeDetector2::setMethod(Method method) { m_method = method; }

void EnvelopeDetector2::setAttackTime(double ms)
{
    m_attackMs = qMax(0.1, ms);
    double frameDur = 1.0 / m_sampleRate;
    m_attackCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_attackMs / 1000.0));
}

void EnvelopeDetector2::setReleaseTime(double ms)
{
    m_releaseMs = qMax(0.1, ms);
    double frameDur = 1.0 / m_sampleRate;
    m_releaseCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_releaseMs / 1000.0));
}

void EnvelopeDetector2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
    double frameDur = 1.0 / m_sampleRate;
    m_attackCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_attackMs / 1000.0));
    m_releaseCoeff = 1.0 - qExp(-frameDur / qMax(0.001, m_releaseMs / 1000.0));
}

/* ---- Helpers ---- */

int EnvelopeDetector2::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- FFT (Cooley-Tukey radix-2) ---- */

void EnvelopeDetector2::fft(QVector<double>& real, QVector<double>& imag,
                              bool inverse) const
{
    int n = real.size();
    if (n <= 1) return;

    /* Bit-reversal */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        double wR = qCos(angle), wI = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tR = cR * real[v] - cI * imag[v];
                double tI = cR * imag[v] + cI * real[v];
                real[v] = real[u] - tR;
                imag[v] = imag[u] - tI;
                real[u] += tR;
                imag[u] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR;
                cR = nR;
            }
        }
    }

    if (inverse)
        for (int i = 0; i < n; ++i) { real[i] /= n; imag[i] /= n; }
}

/* ---- Hilbert transform via FFT ---- */

QVector<double> EnvelopeDetector2::hilbertTransform(const QVector<double>& signal) const
{
    int n = nextPow2(signal.size());
    QVector<double> real(n, 0.0), imag(n, 0.0);

    for (int i = 0; i < signal.size(); ++i) real[i] = signal[i];

    fft(real, imag, false);

    /* Zero negative frequencies, double positive (DC and Nyquist unchanged) */
    for (int i = 1; i < n / 2; ++i) {
        real[i] *= 2.0;
        imag[i] *= 2.0;
    }
    for (int i = n / 2 + 1; i < n; ++i) {
        real[i] = 0.0;
        imag[i] = 0.0;
    }

    fft(real, imag, true);

    /* Return imaginary part (Hilbert transform) */
    QVector<double> result(signal.size());
    for (int i = 0; i < signal.size(); ++i)
        result[i] = imag[i];
    return result;
}

/* ---- Envelope detection (Hilbert method) ---- */

QVector<double> EnvelopeDetector2::detectEnvelope(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n == 0) return QVector<double>();

    QVector<double> hilbert = hilbertTransform(signal);

    /* Envelope = |x(t) + j*h(t)| = sqrt(x² + h²) */
    QVector<double> envelope(n);
    double maxPeak = 0.0;

    for (int i = 0; i < n; ++i) {
        envelope[i] = qSqrt(signal[i] * signal[i] + hilbert[i] * hilbert[i]);
        if (envelope[i] > maxPeak) maxPeak = envelope[i];
    }

    /* Smooth with attack/release */
    QVector<double> smoothed(n);
    double state = 0.0;
    for (int i = 0; i < n; ++i) {
        if (envelope[i] > state)
            state += m_attackCoeff * (envelope[i] - state);
        else
            state += m_releaseCoeff * (envelope[i] - state);
        smoothed[i] = state;
    }

    m_stats.totalSamples += n;
    m_stats.totalFrames++;
    m_stats.lastPeakAmplitude = maxPeak;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit processingCompleted(n);
    return smoothed;
}

/* ---- Peak tracking envelope ---- */

QVector<double> EnvelopeDetector2::detectPeakTrack(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    QVector<double> envelope(n, 0.0);
    double maxPeak = 0.0;
    double state = 0.0;

    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(signal[i]);
        if (absVal > state)
            state += m_attackCoeff * (absVal - state);
        else
            state += m_releaseCoeff * (absVal - state);
        envelope[i] = state;
        if (absVal > maxPeak) maxPeak = absVal;
    }

    m_stats.totalSamples += n;
    m_stats.totalFrames++;
    m_stats.lastPeakAmplitude = maxPeak;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit processingCompleted(n);
    return envelope;
}

/* ---- Instantaneous phase ---- */

QVector<double> EnvelopeDetector2::instantaneousPhase(const QVector<double>& signal)
{
    QVector<double> hilbert = hilbertTransform(signal);
    int n = signal.size();
    QVector<double> phase(n);
    for (int i = 0; i < n; ++i)
        phase[i] = qAtan2(hilbert[i], signal[i]);
    return phase;
}

/* ---- Instantaneous frequency ---- */

QVector<double> EnvelopeDetector2::instantaneousFrequency(const QVector<double>& signal)
{
    QVector<double> phase = instantaneousPhase(signal);
    int n = phase.size();
    if (n == 0) return QVector<double>();

    QVector<double> freq(n);
    freq[0] = 0.0;
    for (int i = 1; i < n; ++i) {
        double dp = phase[i] - phase[i - 1];
        /* Unwrap phase */
        while (dp > M_PI) dp -= 2.0 * M_PI;
        while (dp < -M_PI) dp += 2.0 * M_PI;
        freq[i] = dp * m_sampleRate / (2.0 * M_PI);
    }
    return freq;
}

/* ---- Statistics ---- */

void EnvelopeDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
