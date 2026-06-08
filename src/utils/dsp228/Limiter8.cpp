/**
 * @file Limiter8.cpp
 * @brief Limiter8 实现
 *
 * 实现多频带限幅器：级联前瞻与频带间能量重分配。
 */

#include "utils/dsp228/Limiter8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Limiter8::Limiter8(int sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(qMax(8000, sampleRate))
{
    // Default 4-band configuration
    QVector<BandConfig> defaults(4);
    double nyquist = m_sampleRate / 2.0;
    double bandWidth = nyquist / 4.0;
    for (int i = 0; i < 4; ++i) {
        defaults[i].lowFreq = i * bandWidth;
        defaults[i].highFreq = (i + 1) * bandWidth;
        defaults[i].threshold = -6.0;
        defaults[i].attack = 1.0;
        defaults[i].release = 50.0;
    }
    setBands(defaults);
}

Limiter8::~Limiter8() = default;

/* ---- Set bands ---- */

bool Limiter8::setBands(const QVector<BandConfig>& bands)
{
    if (bands.isEmpty()) return false;
    m_numBands = bands.size();
    m_configs = bands;
    m_bands.resize(m_numBands);

    for (int i = 0; i < m_numBands; ++i) {
        BandState& bs = m_bands[i];
        double threshDb = m_configs[i].threshold;
        bs.thresholdLin = qPow(10.0, threshDb / 20.0);

        // Attack/release coefficients (ms -> samples)
        double attackMs = qMax(0.1, m_configs[i].attack);
        double releaseMs = qMax(1.0, m_configs[i].release);
        bs.attackCoeff = qExp(-1.0 / (m_sampleRate * attackMs / 1000.0));
        bs.releaseCoeff = qExp(-1.0 / (m_sampleRate * releaseMs / 1000.0));

        // Lookahead buffer (proportional to attack time)
        bs.lookaheadSize = qMax(1,
            static_cast<int>(m_sampleRate * attackMs / 1000.0));
        bs.lookaheadBuffer.resize(bs.lookaheadSize, 0.0);
        bs.lookaheadPos = 0;
        bs.envelope = 0.0;
        bs.gain = 1.0;
    }
    m_stats.numBands = m_numBands;
    return true;
}

/* ---- Compute envelope ---- */

double Limiter8::computeEnvelope(double input, BandState& band)
{
    double absVal = qAbs(input);
    if (absVal > band.envelope) {
        band.envelope = band.attackCoeff * band.envelope +
            (1.0 - band.attackCoeff) * absVal;
    } else {
        band.envelope = band.releaseCoeff * band.envelope +
            (1.0 - band.releaseCoeff) * absVal;
    }
    return band.envelope;
}

/* ---- Redistribute energy ---- */

void Limiter8::redistributeEnergy(QVector<double>& gains)
{
    // Compute average gain across bands
    double avgGain = 0.0;
    for (double g : gains) avgGain += g;
    avgGain /= gains.size();

    // Redistribute: push each band's gain toward average
    // to maintain spectral balance
    double blendFactor = 0.3;
    for (int i = 0; i < gains.size(); ++i) {
        double diff = avgGain - gains[i];
        gains[i] += diff * blendFactor;
        gains[i] = qBound(0.0, gains[i], 1.0);
    }
}

/* ---- Apply lookahead ---- */

double Limiter8::applyLookahead(double input, double gain, BandState& band)
{
    // Store in lookahead buffer
    band.lookaheadBuffer[band.lookaheadPos] = input;
    int outputIdx = band.lookaheadPos;
    band.lookaheadPos = (band.lookaheadPos + 1) % band.lookaheadSize;

    // Apply gain to delayed sample
    return band.lookaheadBuffer[outputIdx] * gain;
}

/* ---- Process single sample ---- */

double Limiter8::processSample(double sample)
{
    // Simplified: single-band limiting with cascaded lookahead
    // In full implementation, would split into frequency bands
    double maxGain = 1.0;
    for (int i = 0; i < m_numBands; ++i) {
        double env = computeEnvelope(sample, m_bands[i]);
        if (env > m_bands[i].thresholdLin) {
            double g = m_bands[i].thresholdLin / qMax(1e-10, env);
            m_bands[i].gain = g;
        } else {
            m_bands[i].gain = 1.0;
        }
        if (m_bands[i].gain < maxGain)
            maxGain = m_bands[i].gain;
    }

    QVector<double> gains;
    gains.reserve(m_numBands);
    for (int i = 0; i < m_numBands; ++i)
        gains.append(m_bands[i].gain);

    redistributeEnergy(gains);

    // Apply cascaded lookahead with dominant gain
    double result = applyLookahead(sample, maxGain, m_bands[0]);
    for (int i = 1; i < m_numBands; ++i) {
        result = applyLookahead(result, gains[i], m_bands[i]);
    }
    return result;
}

/* ---- Process block ---- */

QVector<double> Limiter8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    double inputPeak = 0.0;
    double outputPeak = 0.0;
    QVector<double> output(n);

    for (int i = 0; i < n; ++i) {
        double absIn = qAbs(input[i]);
        if (absIn > inputPeak) inputPeak = absIn;

        output[i] = processSample(input[i]);

        double absOut = qAbs(output[i]);
        if (absOut > outputPeak) outputPeak = absOut;
    }

    double reductionDb = (inputPeak > 0 && outputPeak > 0)
        ? 20.0 * qLn(outputPeak / inputPeak) / qLn(10.0) : 0.0;

    m_stats.numSamples += n;
    m_stats.peakReductionDb = reductionDb;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(n, reductionDb, timer.elapsed());
    return output;
}

/* ---- Gain reduction per band ---- */

QVector<double> Limiter8::gainReduction() const
{
    QVector<double> gr;
    gr.reserve(m_numBands);
    for (int i = 0; i < m_numBands; ++i) {
        double grDb = 20.0 * qLn(qMax(1e-10, m_bands[i].gain)) / qLn(10.0);
        gr.append(grDb);
    }
    return gr;
}

/* ---- Reset ---- */

void Limiter8::reset()
{
    for (int i = 0; i < m_numBands; ++i) {
        m_bands[i].envelope = 0.0;
        m_bands[i].gain = 1.0;
        m_bands[i].lookaheadBuffer.fill(0.0);
        m_bands[i].lookaheadPos = 0;
    }
}

/* ---- Reset statistics ---- */

void Limiter8::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
