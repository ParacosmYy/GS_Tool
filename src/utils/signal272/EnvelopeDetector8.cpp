/**
 * @file EnvelopeDetector8.cpp
 * @brief EnvelopeDetector8 实现
 *
 * 实现包络检测器：Hilbert变换解析信号与峰值保持衰减瞬时幅度跟踪。
 */

#include "utils/signal272/EnvelopeDetector8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector8::EnvelopeDetector8(QObject *parent)
    : QObject(parent) {}

EnvelopeDetector8::~EnvelopeDetector8() = default;

/* ---- Configuration ---- */

void EnvelopeDetector8::setMode(Mode mode)
{
    m_mode = mode;
}

void EnvelopeDetector8::setAttackMs(double ms)
{
    m_attackMs = qBound(0.01, ms, 100.0);
}

void EnvelopeDetector8::setReleaseMs(double ms)
{
    m_releaseMs = qBound(1.0, ms, 5000.0);
}

void EnvelopeDetector8::setSampleRate(double rate)
{
    m_sampleRate = qBound(8000.0, rate, 192000.0);
}

void EnvelopeDetector8::setFilterLength(int len)
{
    // Ensure odd length for symmetric Hilbert FIR
    if (len % 2 == 0) len++;
    m_filterLen = qBound(15, len, 511);
}

/* ---- Design Hilbert FIR coefficients ---- */

QVector<double> EnvelopeDetector8::hilbertCoeffs() const
{
    int M = m_filterLen - 1;  // filter order
    int halfLen = M / 2;
    QVector<double> h(m_filterLen, 0.0);

    for (int n = 0; n <= M; ++n) {
        if (n == halfLen) {
            h[n] = 0.0;  // center tap is zero for Hilbert
            continue;
        }
        // Ideal Hilbert impulse response
        double k = n - halfLen;
        h[n] = (1.0 - qCos(M_PI * k)) / (M_PI * k);

        // Apply Hamming window
        double window = 0.54 - 0.46 * qCos(2.0 * M_PI * n / M);
        h[n] *= window;
    }

    return h;
}

/* ---- Apply FIR filter ---- */

QVector<double> EnvelopeDetector8::applyFIR(const QVector<double>& input,
                                             const QVector<double>& coeffs) const
{
    int n = input.size();
    int tap = coeffs.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double acc = 0.0;
        for (int j = 0; j < tap; ++j) {
            int idx = i - j + tap / 2;
            if (idx >= 0 && idx < n)
                acc += coeffs[j] * input[idx];
        }
        output[i] = acc;
    }
    return output;
}

/* ---- Hilbert transform ---- */

QVector<double> EnvelopeDetector8::hilbertTransform(const QVector<double>& input) const
{
    QVector<double> coeffs = hilbertCoeffs();
    return applyFIR(input, coeffs);
}

/* ---- Peak-hold with exponential decay ---- */

QVector<double> EnvelopeDetector8::peakHoldDecay(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> env(n, 0.0);

    double attackCoeff = qExp(-1.0 / (m_attackMs * m_sampleRate / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * m_sampleRate / 1000.0));
    double prev = m_prevEnv;

    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(input[i]);
        if (absVal > prev) {
            // Attack
            prev = attackCoeff * prev + (1.0 - attackCoeff) * absVal;
        } else {
            // Release
            prev = releaseCoeff * prev + (1.0 - releaseCoeff) * absVal;
        }
        env[i] = prev;
    }
    return env;
}

/* ---- Full-wave rectification + smoothing ---- */

QVector<double> EnvelopeDetector8::rectifySmooth(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> rectified(n);
    for (int i = 0; i < n; ++i)
        rectified[i] = qAbs(input[i]);

    // Simple single-pole low-pass filter
    double alpha = 1.0 - qExp(-1.0 / (m_releaseMs * m_sampleRate / 1000.0));
    QVector<double> env(n, 0.0);
    env[0] = rectified[0];

    for (int i = 1; i < n; ++i)
        env[i] = env[i - 1] + alpha * (rectified[i] - env[i - 1]);

    return env;
}

/* ---- Main processing ---- */

QVector<double> EnvelopeDetector8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return input;

    switch (m_mode) {
    case HilbertAnalytic: {
        // Compute Hilbert transform (imaginary part of analytic signal)
        QVector<double> hilbert = hilbertTransform(input);

        // Envelope = |x(t) + j*h(t)| = sqrt(x^2 + h^2)
        m_envelope.resize(n);
        for (int i = 0; i < n; ++i)
            m_envelope[i] = qSqrt(input[i] * input[i] + hilbert[i] * hilbert[i]);
        break;
    }
    case PeakHoldDecay:
        m_envelope = peakHoldDecay(input);
        break;
    case RectifySmooth:
        m_envelope = rectifySmooth(input);
        break;
    }

    // Update state for continuity
    if (!m_envelope.isEmpty())
        m_prevEnv = m_envelope.last();

    // Compute statistics
    double peakEnv = 0.0;
    double rmsSum = 0.0;
    for (int i = 0; i < n; ++i) {
        if (m_envelope[i] > peakEnv) peakEnv = m_envelope[i];
        rmsSum += m_envelope[i] * m_envelope[i];
    }
    double rmsEnv = qSqrt(rmsSum / n);

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.peakEnvelope = peakEnv;
    m_stats.rmsEnvelope = rmsEnv;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit detectionCompleted(n, peakEnv, elapsed);

    return m_envelope;
}

/* ---- Accessors ---- */

QVector<double> EnvelopeDetector8::envelope() const { return m_envelope; }

/* ---- Reset ---- */

void EnvelopeDetector8::resetStatistics()
{
    m_envelope.clear();
    m_prevEnv = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
