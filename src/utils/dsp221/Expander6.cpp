/**
 * @file Expander6.cpp
 * @brief Expander6 实现
 *
 * 实现多频段扩展器：对数频段划分、瞬态检测、心理声学门控。
 */

#include "utils/dsp221/Expander6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander6::Expander6(QObject *parent) : QObject(parent) {}
Expander6::~Expander6() = default;

/* ---- Configuration ---- */

void Expander6::setParameters(int sampleRate, int blockSize)
{
    m_sampleRate = qMax(8000, sampleRate);
    m_blockSize = qMax(64, blockSize);
    m_stats.sampleRate = m_sampleRate;
    m_stats.blockSize = m_blockSize;
}

void Expander6::configureBands(const QVector<BandConfig>& bands)
{
    m_bands = bands;
    m_bandStates.resize(bands.size());
    m_lowCoeffs.resize(bands.size());
    m_highCoeffs.resize(bands.size());
    m_lowStates.resize(bands.size());
    m_highStates.resize(bands.size());

    for (int i = 0; i < bands.size(); ++i) {
        if (bands[i].lowFreq > 0)
            m_highCoeffs[i] = highpassCoeffs(bands[i].lowFreq);
        if (bands[i].highFreq > 0 && bands[i].highFreq < m_sampleRate / 2.0)
            m_lowCoeffs[i] = lowpassCoeffs(bands[i].highFreq);
        m_bandStates[i] = BandState{};
    }
    m_stats.numBands = bands.size();
}

/* ---- Auto-generate logarithmic bands ---- */

QVector<Expander6::BandConfig> Expander6::generateLogBands(
    int numBands, double minFreq, double maxFreq) const
{
    QVector<BandConfig> bands(numBands);
    double logMin = qLn(qMax(1.0, minFreq));
    double logMax = qLn(qMax(2.0, maxFreq));
    double step = (logMax - logMin) / numBands;

    for (int i = 0; i < numBands; ++i) {
        bands[i].lowFreq = qExp(logMin + i * step);
        bands[i].highFreq = qExp(logMin + (i + 1) * step);
        bands[i].threshold = -40.0 + i * 5.0;
        bands[i].ratio = 2.0;
        bands[i].attack = 5.0;
        bands[i].release = 50.0;
        bands[i].makeupGain = 0.0;
    }
    return bands;
}

/* ---- Biquad filter design ---- */

Expander6::BiquadCoeffs Expander6::lowpassCoeffs(double cutoffFreq) const
{
    BiquadCoeffs c;
    double w0 = 2.0 * M_PI * cutoffFreq / m_sampleRate;
    double alpha = qSin(w0) / (2.0 * 0.7071); // Q = 0.7071 (Butterworth)
    double cosw = qCos(w0);

    double a0 = 1.0 + alpha;
    c.b0 = (1.0 - cosw) / (2.0 * a0);
    c.b1 = (1.0 - cosw) / a0;
    c.b2 = (1.0 - cosw) / (2.0 * a0);
    c.a1 = -2.0 * cosw / a0;
    c.a2 = (1.0 - alpha) / a0;
    return c;
}

Expander6::BiquadCoeffs Expander6::highpassCoeffs(double cutoffFreq) const
{
    BiquadCoeffs c;
    double w0 = 2.0 * M_PI * cutoffFreq / m_sampleRate;
    double alpha = qSin(w0) / (2.0 * 0.7071);
    double cosw = qCos(w0);

    double a0 = 1.0 + alpha;
    c.b0 = (1.0 + cosw) / (2.0 * a0);
    c.b1 = -(1.0 + cosw) / a0;
    c.b2 = (1.0 + cosw) / (2.0 * a0);
    c.a1 = -2.0 * cosw / a0;
    c.a2 = (1.0 - alpha) / a0;
    return c;
}

/* ---- Apply biquad ---- */

double Expander6::applyBiquad(double sample, const BiquadCoeffs& c,
                                FilterState& s) const
{
    double y = c.b0 * sample + c.b1 * s.x1 + c.b2 * s.x2
               - c.a1 * s.y1 - c.a2 * s.y2;
    s.x2 = s.x1; s.x1 = sample;
    s.y2 = s.y1; s.y1 = y;
    return y;
}

/* ---- Envelope computation ---- */

double Expander6::computeEnvelope(double input, double attack, double release,
                                    double state) const
{
    double coeff = (input > state)
        ? qExp(-1.0 / (m_sampleRate * attack * 0.001))
        : qExp(-1.0 / (m_sampleRate * release * 0.001));
    return input + (1.0 - coeff) * (state - input);
}

/* ---- Transient detection ---- */

double Expander6::detectTransient(const QVector<double>& bandSignal) const
{
    if (bandSignal.size() < 2) return 0.0;
    double flux = 0.0;
    for (int i = 1; i < bandSignal.size(); ++i) {
        double diff = qAbs(bandSignal[i]) - qAbs(bandSignal[i - 1]);
        if (diff > 0) flux += diff;
    }
    return flux / bandSignal.size();
}

/* ---- Psychoacoustic threshold ---- */

double Expander6::psychoThreshold(double freq, double level) const
{
    // Simplified equal-loudness approximation
    double f2 = freq * freq;
    double thresh = 3.64 * qPow(freq / 1000.0, -0.8)
                    - 6.5 * qExp(-0.6 * qPow((freq / 1000.0 - 3.3), 2))
                    + 1e-3 * qPow(freq / 1000.0, 4);
    return level + thresh;
}

/* ---- Process block ---- */

QVector<double> Expander6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), m_blockSize);
    QVector<double> output(n, 0.0);
    int activeBands = 0;
    double peakReduction = 0.0;

    for (int b = 0; b < m_bands.size(); ++b) {
        // Bandpass filter the signal
        QVector<double> bandSig(n);
        for (int i = 0; i < n; ++i) {
            double s = input[i];
            if (m_bands[b].highFreq > 0)
                s = applyBiquad(s, m_lowCoeffs[b], m_lowStates[b]);
            if (m_bands[b].lowFreq > 0)
                s = applyBiquad(s, m_highCoeffs[b], m_highStates[b]);
            bandSig[i] = s;
        }

        // Detect transient
        double transient = detectTransient(bandSig);
        m_bandStates[b].transientLevel = transient;

        // Compute envelope
        double env = 0.0;
        for (int i = 0; i < n; ++i) {
            double absVal = qAbs(bandSig[i]);
            env = computeEnvelope(absVal, m_bands[b].attack,
                                  m_bands[b].release, env);
        }
        m_bandStates[b].envelope = env;

        // Convert to dB
        double envDb = (env > 1e-10) ? 20.0 * qLn(env) / qLn(10.0) : -120.0;

        // Psychoacoustic gate: adjust threshold based on transient
        double effThresh = m_bands[b].threshold;
        if (transient > 0.01) {
            double bandCenter = (m_bands[b].lowFreq + m_bands[b].highFreq) / 2.0;
            effThresh = psychoThreshold(bandCenter, effThresh);
        }

        // Compute expansion gain
        double gainDb = 0.0;
        if (envDb < effThresh) {
            double overDb = effThresh - envDb;
            gainDb = -overDb * (1.0 - 1.0 / m_bands[b].ratio);
            m_bandStates[b].gated = true;
            activeBands++;
        } else {
            m_bandStates[b].gated = false;
        }
        gainDb += m_bands[b].makeupGain;
        peakReduction = qMin(peakReduction, gainDb);

        double gainLin = qPow(10.0, gainDb / 20.0);
        m_bandStates[b].gainLin = gainLin;

        // Apply gain and accumulate
        for (int i = 0; i < n; ++i)
            output[i] += bandSig[i] * gainLin;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit blockProcessed(activeBands, peakReduction, timer.elapsed());
    return output;
}

/* ---- Band states ---- */

QVector<Expander6::BandState> Expander6::bandStates() const
{
    return m_bandStates;
}

/* ---- Reset ---- */

void Expander6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bandStates.clear();
    m_lowStates.clear();
    m_highStates.clear();
}
