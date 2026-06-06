/**
 * @file EnvelopeDetector3.cpp
 * @brief EnvelopeDetector3 实现
 *
 * 实现包络检测：Hilbert变换、解析信号、瞬时相位/频率。
 */

#include "utils/signal186/EnvelopeDetector3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector3::EnvelopeDetector3(QObject *parent) : QObject(parent) {}
EnvelopeDetector3::~EnvelopeDetector3() = default;

/* ---- Helpers ---- */

int EnvelopeDetector3::nextPow2(int n) const
{
    int p = 1;
    while (p < n) p *= 2;
    return p;
}

/* ---- FFT ---- */

void EnvelopeDetector3::fft(QVector<double>& re, QVector<double>& im,
                              bool inverse) const
{
    int N = re.size();
    if (N <= 1) return;
    int log2N = 0;
    while ((1 << log2N) < N) ++log2N;

    for (int i = 0; i < N; ++i) {
        int j = 0;
        for (int b = 0; b < log2N; ++b) j = (j << 1) | ((i >> b) & 1);
        if (j > i) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    double sign = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= N; len *= 2) {
        double ang = sign * 2.0 * M_PI / len;
        double wR = qCos(ang), wI = qSin(ang);
        for (int i = 0; i < N; i += len) {
            double cR = 1.0, cI = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int e = i + j, o = i + j + len / 2;
                double tR = cR * re[o] - cI * im[o];
                double tI = cR * im[o] + cI * re[o];
                re[o] = re[e] - tR; im[o] = im[e] - tI;
                re[e] += tR; im[e] += tI;
                double nR = cR * wR - cI * wI;
                cI = cR * wI + cI * wR; cR = nR;
            }
        }
    }
    if (inverse) for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }
}

/* ---- Hilbert transform ---- */

QVector<double> EnvelopeDetector3::hilbertTransform(
    const QVector<double>& signal) const
{
    int n = signal.size();
    if (n == 0) return {};

    int N = nextPow2(n);
    QVector<double> re(N, 0.0), im(N, 0.0);
    for (int i = 0; i < n; ++i) re[i] = signal[i];

    fft(re, im, false);

    // Multiply positive frequencies by -j (rotate -90 degrees)
    // and negative frequencies by +j (rotate +90 degrees)
    for (int k = 0; k < N; ++k) {
        if (k == 0 || k == N / 2) {
            // DC and Nyquist: zero out
            re[k] = 0.0;
            im[k] = 0.0;
        } else if (k < N / 2) {
            // Positive freq: multiply by -j => (re+j*im)*(-j) = im - j*re
            double r = re[k], i = im[k];
            re[k] = i;
            im[k] = -r;
        } else {
            // Negative freq: multiply by +j => (re+j*im)*(j) = -im + j*re
            double r = re[k], i = im[k];
            re[k] = -i;
            im[k] = r;
        }
    }

    fft(re, im, true);
    return re.mid(0, n);
}

/* ---- Analytic signal ---- */

void EnvelopeDetector3::analyticSignal(const QVector<double>& signal,
                                         QVector<double>& realPart,
                                         QVector<double>& imagPart) const
{
    int n = signal.size();
    if (n == 0) { realPart.clear(); imagPart.clear(); return; }

    int N = nextPow2(n);
    QVector<double> re(N, 0.0), im(N, 0.0);
    for (int i = 0; i < n; ++i) re[i] = signal[i];

    fft(re, im, false);

    // Build analytic signal: double positive freq, zero negative freq
    for (int k = 1; k < N / 2; ++k) {
        re[k] *= 2.0;
        im[k] *= 2.0;
    }
    for (int k = N / 2 + 1; k < N; ++k) {
        re[k] = 0.0;
        im[k] = 0.0;
    }
    // DC and Nyquist stay as-is

    fft(re, im, true);

    realPart = re.mid(0, n);
    imagPart = im.mid(0, n);
}

/* ---- Envelope ---- */

QVector<double> EnvelopeDetector3::envelope(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re, im;
    analyticSignal(signal, re, im);

    int n = re.size();
    QVector<double> env(n);
    for (int i = 0; i < n; ++i)
        env[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);

    m_stats.totalDetections++;
    m_stats.signalLength = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(n, timer.elapsed());
    return env;
}

/* ---- Instantaneous phase ---- */

QVector<double> EnvelopeDetector3::instantaneousPhase(
    const QVector<double>& signal) const
{
    QVector<double> re, im;
    analyticSignal(signal, re, im);

    int n = re.size();
    QVector<double> phase(n);
    for (int i = 0; i < n; ++i)
        phase[i] = qAtan2(im[i], re[i]);
    return phase;
}

/* ---- Instantaneous frequency ---- */

QVector<double> EnvelopeDetector3::instantaneousFrequency(
    const QVector<double>& signal, double sampleRate) const
{
    QVector<double> phase = instantaneousPhase(signal);
    int n = phase.size();
    if (n == 0) return {};

    QVector<double> freq(n, 0.0);

    // Unwrap phase and compute derivative
    for (int i = 1; i < n; ++i) {
        double diff = phase[i] - phase[i - 1];
        // Phase unwrapping
        while (diff > M_PI) diff -= 2.0 * M_PI;
        while (diff < -M_PI) diff += 2.0 * M_PI;
        freq[i] = diff * sampleRate / (2.0 * M_PI);
    }
    freq[0] = (n > 1) ? freq[1] : 0.0;

    return freq;
}

/* ---- Reset ---- */

void EnvelopeDetector3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
