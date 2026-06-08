/**
 * @file MultibandGate4.cpp
 * @brief MultibandGate4 实现
 *
 * 实现多频段门控：常量Q变换频带分解与独立包络跟随。
 */

#include "utils/dsp234/MultibandGate4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandGate4::MultibandGate4(QObject *parent) : QObject(parent) {}
MultibandGate4::~MultibandGate4() = default;

/* ---- Configuration ---- */

bool MultibandGate4::configure(int frameSize, int sampleRate, int numBands)
{
    if (frameSize < 8 || sampleRate < 1000 || numBands < 1) return false;

    m_frameSize = frameSize;
    m_sampleRate = sampleRate;
    m_numBands = numBands;

    m_stats.frameSize = frameSize;
    m_stats.sampleRate = sampleRate;
    m_stats.numBands = numBands;

    computeCQTBands();
    computeWindow();
    return true;
}

/* ---- Setters ---- */

void MultibandGate4::setBandThreshold(int band, double thresholdDb)
{
    if (band >= 0 && band < m_bands.size())
        m_bands[band].threshold = thresholdDb;
}

void MultibandGate4::setAttack(double ms)
{
    double tau = ms / 1000.0;
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * tau));
}

void MultibandGate4::setRelease(double ms)
{
    double tau = ms / 1000.0;
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * tau));
}

/* ---- Compute CQT bands ---- */

void MultibandGate4::computeCQTBands()
{
    // Constant-Q: geometric frequency spacing
    // Q = f_center / bandwidth, keep Q constant across bands
    double fMin = 60.0;   // Minimum center frequency
    double fMax = m_sampleRate / 2.0 * 0.9;
    double ratio = qPow(fMax / fMin, 1.0 / m_numBands);

    m_bands.resize(m_numBands);
    m_cqtKernel.resize(m_frameSize * m_numBands);

    for (int b = 0; b < m_numBands; ++b) {
        double fc = fMin * qPow(ratio, b);
        double bw = fc * (ratio - 1.0);
        m_bands[b].centerFreq = fc;
        m_bands[b].bandwidth = bw;
        m_bands[b].threshold = -40.0;
        m_bands[b].gain = 0.0;
        m_bands[b].isOpen = false;
        m_bands[b].envelope = 0.0;

        // Compute CQT kernel (complex exponential) for this band
        int halfN = m_frameSize / 2;
        double k = fc * m_frameSize / m_sampleRate;
        for (int n = 0; n < m_frameSize; ++n) {
            double angle = 2.0 * M_PI * k * n / m_frameSize;
            m_cqtKernel[b * m_frameSize + n] = qCos(angle);
        }
    }
}

/* ---- Compute window ---- */

void MultibandGate4::computeWindow()
{
    m_window.resize(m_frameSize);
    for (int n = 0; n < m_frameSize; ++n) {
        m_window[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / m_frameSize));
    }
}

/* ---- CQT decomposition ---- */

QVector<double> MultibandGate4::cqtDecompose(const QVector<double>& frame) const
{
    QVector<double> magnitudes(m_numBands, 0.0);
    for (int b = 0; b < m_numBands; ++b) {
        double re = 0.0, im = 0.0;
        double k = m_bands[b].centerFreq * m_frameSize / m_sampleRate;
        for (int n = 0; n < qMin(frame.size(), m_frameSize); ++n) {
            double angle = 2.0 * M_PI * k * n / m_frameSize;
            re += frame[n] * m_window[n] * qCos(angle);
            im += frame[n] * m_window[n] * qSin(angle);
        }
        magnitudes[b] = qSqrt(re * re + im * im) / m_frameSize;
    }
    return magnitudes;
}

/* ---- CQT reconstruction ---- */

QVector<double> MultibandGate4::cqtReconstruct(const QVector<double>& frame,
                                                 const QVector<double>& gains) const
{
    QVector<double> output(frame.size(), 0.0);
    for (int b = 0; b < m_numBands; ++b) {
        double k = m_bands[b].centerFreq * m_frameSize / m_sampleRate;
        // Filter around center frequency using gain
        for (int n = 0; n < qMin(frame.size(), m_frameSize); ++n) {
            double angle = 2.0 * M_PI * k * n / m_frameSize;
            // Simple bandpass: modulate with gain
            output[n] += frame[n] * m_window[n] * gains[b] * qCos(angle);
        }
    }
    // Normalize
    for (int n = 0; n < output.size(); ++n)
        output[n] /= qMax(1, m_numBands);
    return output;
}

/* ---- Envelope update ---- */

double MultibandGate4::updateEnvelope(double current, double sample) const
{
    double coeff = (sample > current) ? m_attackCoeff : m_releaseCoeff;
    return current + coeff * (sample - current);
}

/* ---- Gate gain ---- */

double MultibandGate4::computeGateGain(double envelope, double threshold) const
{
    // Convert threshold from dB to linear
    double threshLin = qPow(10.0, threshold / 20.0);
    if (envelope >= threshLin) return 1.0;
    // Below threshold: apply attenuation
    double ratio = envelope / qMax(1e-10, threshLin);
    return ratio * ratio;  // Quadratic fade
}

/* ---- Process frame ---- */

QVector<double> MultibandGate4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    // Decompose into bands
    QVector<double> magnitudes = cqtDecompose(input);

    // Update envelope followers and compute gains
    QVector<double> gains(m_numBands);
    int activeBands = 0;
    for (int b = 0; b < m_numBands; ++b) {
        double env = updateEnvelope(m_bands[b].envelope, magnitudes[b]);
        m_bands[b].envelope = env;
        gains[b] = computeGateGain(env, m_bands[b].threshold);
        m_bands[b].gain = gains[b];
        bool wasOpen = m_bands[b].isOpen;
        m_bands[b].isOpen = (gains[b] > 0.5);
        if (m_bands[b].isOpen) activeBands++;
        if (wasOpen != m_bands[b].isOpen)
            emit bandStateChanged(b, m_bands[b].isOpen);
    }

    // Reconstruct output with band gains
    QVector<double> output = cqtReconstruct(input, gains);

    m_frameNum++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit frameProcessed(m_frameNum, activeBands);
    return output;
}

/* ---- Accessors ---- */

QVector<MultibandGate4::BandState> MultibandGate4::bandStates() const { return m_bands; }

/* ---- Reset ---- */

void MultibandGate4::resetStatistics()
{
    m_bands.clear();
    m_cqtKernel.clear();
    m_window.clear();
    m_frameNum = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
