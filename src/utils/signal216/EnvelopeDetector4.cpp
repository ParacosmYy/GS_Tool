/**
 * @file EnvelopeDetector4.cpp
 * @brief EnvelopeDetector4 实现
 *
 * 实现包络检测：希尔伯特变换、解析信号、对数域RMS平滑。
 */

#include "utils/signal216/EnvelopeDetector4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EnvelopeDetector4::EnvelopeDetector4(QObject *parent) : QObject(parent)
{
    designHilbertFilter();
}

EnvelopeDetector4::~EnvelopeDetector4() = default;

/* ---- Design Hilbert FIR filter ---- */

void EnvelopeDetector4::designHilbertFilter(int order)
{
    // Ensure odd length for type-III FIR
    if (order % 2 == 0) ++order;
    int M = (order - 1) / 2;
    m_hilbertCoeffs.resize(order);
    m_hilbertDelay = M;

    for (int n = 0; n < order; ++n) {
        int k = n - M;
        if (k == 0) {
            m_hilbertCoeffs[n] = 0.0;
        } else {
            // Ideal Hilbert: h[k] = 2/(pi*k) for k odd, 0 for k even
            if (k % 2 != 0)
                m_hilbertCoeffs[n] = 2.0 / (M_PI * k);
            else
                m_hilbertCoeffs[n] = 0.0;
        }
        // Apply Hann window
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (order - 1)));
        m_hilbertCoeffs[n] *= w;
    }

    m_history.resize(order, 0.0);
    m_histPos = 0;
}

/* ---- Configuration ---- */

void EnvelopeDetector4::setParameters(double attackMs, double releaseMs,
                                       double sampleRate)
{
    m_sampleRate = qMax(8000.0, sampleRate);
    double attackSamples = attackMs * m_sampleRate / 1000.0;
    double releaseSamples = releaseMs * m_sampleRate / 1000.0;

    m_attackCoeff = (attackSamples > 0.0) ? qExp(-1.0 / attackSamples) : 0.0;
    m_releaseCoeff = (releaseSamples > 0.0) ? qExp(-1.0 / releaseSamples) : 0.0;

    m_stats.attackMs = attackMs;
    m_stats.releaseMs = releaseMs;
    m_stats.sampleRate = m_sampleRate;
}

/* ---- Hilbert transform via FIR ---- */

void EnvelopeDetector4::hilbertTransform(const QVector<double>& input,
                                          QVector<double>& analyticReal,
                                          QVector<double>& analyticImag) const
{
    int n = input.size();
    int order = m_hilbertCoeffs.size();
    analyticReal = input;
    analyticImag.resize(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < order; ++k) {
            int idx = i - m_hilbertDelay + k;
            if (idx >= 0 && idx < n)
                sum += input[idx] * m_hilbertCoeffs[k];
        }
        analyticImag[i] = sum;
    }
}

/* ---- Envelope from analytic signal ---- */

QVector<double> EnvelopeDetector4::envelopeFromAnalytic(
    const QVector<double>& real, const QVector<double>& imag) const
{
    int n = qMin(real.size(), imag.size());
    QVector<double> env(n);
    for (int i = 0; i < n; ++i)
        env[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
    return env;
}

/* ---- Log-domain RMS smoothing ---- */

double EnvelopeDetector4::smoothLogRMS(double currentEnvelope, double newLevel)
{
    // Work in log domain for better dynamic range
    double logCurrent = (currentEnvelope > 1e-10)
        ? 20.0 * qLn(currentEnvelope) / M_LN10 : -200.0;
    double logNew = (newLevel > 1e-10)
        ? 20.0 * qLn(newLevel) / M_LN10 : -200.0;

    double coeff = (logNew > logCurrent) ? m_attackCoeff : m_releaseCoeff;
    m_logRmsState = coeff * logCurrent + (1.0 - coeff) * logNew;

    // Back to linear
    return qPow(10.0, m_logRmsState / 20.0);
}

/* ---- Process single sample ---- */

double EnvelopeDetector4::processOne(double input)
{
    // Update circular buffer
    m_history[m_histPos] = input;

    // Compute Hilbert imaginary part via FIR
    double imag = 0.0;
    int order = m_hilbertCoeffs.size();
    for (int k = 0; k < order; ++k) {
        int idx = (m_histPos - k + order) % order;
        imag += m_history[idx] * m_hilbertCoeffs[order - 1 - k];
    }

    // Delayed real part (to align with Hilbert filter group delay)
    int delayIdx = (m_histPos - m_hilbertDelay + order) % order;
    double realPart = m_history[delayIdx];

    // Envelope = magnitude of analytic signal
    double raw = qSqrt(realPart * realPart + imag * imag);

    // Apply log-domain smoothing
    m_envelope = smoothLogRMS(m_envelope, raw);

    m_histPos = (m_histPos + 1) % order;
    return m_envelope;
}

/* ---- Process buffer ---- */

QVector<double> EnvelopeDetector4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double peakEnv = 0.0;

    for (int i = 0; i < n; ++i) {
        output[i] = processOne(input[i]);
        peakEnv = qMax(peakEnv, output[i]);
    }

    m_stats.totalSamples += n;
    m_stats.peakEnvelope = peakEnv;
    m_stats.rmsEnvelope = m_envelope;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples);
    emit processingCompleted(n, peakEnv, timer.elapsed());

    return output;
}

/* ---- Current envelope ---- */

double EnvelopeDetector4::currentEnvelope() const
{
    return m_envelope;
}

/* ---- Reset ---- */

void EnvelopeDetector4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 0.0;
    m_logRmsState = 0.0;
    m_history.fill(0.0);
    m_histPos = 0;
}
