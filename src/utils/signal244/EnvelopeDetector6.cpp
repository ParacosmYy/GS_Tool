/**
 * @file EnvelopeDetector6.cpp
 * @brief EnvelopeDetector6 实现
 *
 * 实现包络检测器：Hilbert变换解析信号与峰值保持衰减可配置时间常数。
 */

#include "utils/signal244/EnvelopeDetector6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector6::EnvelopeDetector6(QObject *parent) : QObject(parent) {}
EnvelopeDetector6::~EnvelopeDetector6() = default;

/* ---- Configuration ---- */

void EnvelopeDetector6::setDecayTimeConstant(double ms) { m_decayMs = qMax(0.1, ms); }
void EnvelopeDetector6::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void EnvelopeDetector6::setFFTSize(int n) { m_fftSize = qMax(4, n); }
void EnvelopeDetector6::setMode(int mode) { m_mode = qBound(0, mode, 1); }

/* ---- Radix-2 FFT ---- */

void EnvelopeDetector6::fft(QVector<double>& re, QVector<double>& im, int sign) const
{
    int n = re.size();
    // Bit-reversal
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = sign * 2.0 * M_PI / len;
        double wRe = qCos(ang), wIm = qSin(ang);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe;
                im[v] = im[u] - tIm;
                re[u] += tRe;
                im[u] += tIm;
                double nRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = nRe;
            }
        }
    }
}

/* ---- Hilbert transform via FFT ---- */

void EnvelopeDetector6::hilbertTransform(const QVector<double>& in,
                                          QVector<double>& outRe,
                                          QVector<double>& outIm) const
{
    int n = in.size();
    int N = 1;
    while (N < n) N <<= 1;

    QVector<double> re(N, 0.0), im(N, 0.0);
    for (int i = 0; i < n; ++i) re[i] = in[i];

    // Forward FFT
    fft(re, im, -1);

    // Zero negative frequencies, double positive
    for (int i = 1; i < N / 2; ++i) {
        re[i] *= 2.0;
        im[i] *= 2.0;
    }
    // DC and Nyquist stay
    for (int i = N / 2 + 1; i < N; ++i) {
        re[i] = 0.0;
        im[i] = 0.0;
    }

    // Inverse FFT
    fft(re, im, 1);
    for (int i = 0; i < N; ++i) { re[i] /= N; im[i] /= N; }

    outRe.resize(n);
    outIm.resize(n);
    for (int i = 0; i < n; ++i) {
        outRe[i] = re[i];
        outIm[i] = im[i];
    }
}

/* ---- Peak-hold decay ---- */

QVector<double> EnvelopeDetector6::peakHoldDecay(const QVector<double>& envelope) const
{
    int n = envelope.size();
    QVector<double> result(n, 0.0);
    if (n == 0) return result;

    double alpha = qExp(-1.0 / (m_decayMs * 0.001 * m_sampleRate));
    double peak = 0.0;

    for (int i = 0; i < n; ++i) {
        if (envelope[i] > peak)
            peak = envelope[i];
        else
            peak *= alpha;
        result[i] = peak;
    }
    return result;
}

/* ---- Unwrap phase ---- */

QVector<double> EnvelopeDetector6::unwrapPhase(const QVector<double>& phase)
{
    int n = phase.size();
    QVector<double> unwrapped(n, 0.0);
    if (n == 0) return unwrapped;

    unwrapped[0] = phase[0];
    for (int i = 1; i < n; ++i) {
        double diff = phase[i] - phase[i - 1];
        while (diff > M_PI) diff -= 2.0 * M_PI;
        while (diff < -M_PI) diff += 2.0 * M_PI;
        unwrapped[i] = unwrapped[i - 1] + diff;
    }
    return unwrapped;
}

/* ---- Process ---- */

QVector<double> EnvelopeDetector6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> envelope(n, 0.0);

    if (m_mode == 0) {
        // Hilbert transform mode
        QVector<double> analyticRe, analyticIm;
        hilbertTransform(input, analyticRe, analyticIm);

        for (int i = 0; i < n; ++i)
            envelope[i] = qSqrt(analyticRe[i] * analyticRe[i] + analyticIm[i] * analyticIm[i]);

        // Compute phase for frequency estimation
        m_lastPhase.resize(n);
        for (int i = 0; i < n; ++i)
            m_lastPhase[i] = qAtan2(analyticIm[i], analyticRe[i]);
    } else {
        // Peak-hold mode: compute |x| then apply decay
        for (int i = 0; i < n; ++i)
            envelope[i] = qAbs(input[i]);
        envelope = peakHoldDecay(envelope);
    }

    double peakEnv = 0.0, sumEnv = 0.0;
    for (int i = 0; i < n; ++i) {
        if (envelope[i] > peakEnv) peakEnv = envelope[i];
        sumEnv += envelope[i];
    }

    m_stats.numSamples = n;
    m_stats.peakEnvelope = peakEnv;
    m_stats.avgEnvelope = (n > 0) ? sumEnv / n : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, peakEnv, timer.elapsed());
    return envelope;
}

/* ---- Instantaneous phase ---- */

QVector<double> EnvelopeDetector6::instantaneousPhase() const
{
    return unwrapPhase(m_lastPhase);
}

/* ---- Instantaneous frequency ---- */

QVector<double> EnvelopeDetector6::instantaneousFrequency() const
{
    QVector<double> phase = unwrapPhase(m_lastPhase);
    int n = phase.size();
    QVector<double> freq(n, 0.0);
    for (int i = 1; i < n; ++i)
        freq[i] = (phase[i] - phase[i - 1]) * m_sampleRate / (2.0 * M_PI);
    if (n > 0) freq[0] = (n > 1) ? freq[1] : 0.0;
    return freq;
}

/* ---- Reset ---- */

void EnvelopeDetector6::resetStatistics()
{
    m_lastPhase.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
