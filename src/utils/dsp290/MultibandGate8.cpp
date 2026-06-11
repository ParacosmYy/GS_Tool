/**
 * @file MultibandGate8.cpp
 * @brief MultibandGate8 实现
 *
 * 实现多频段门：交叉滤波器组与逐频段滞后控制实现频率选择性噪声抑制。
 */

#include "utils/dsp290/MultibandGate8.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

MultibandGate8::MultibandGate8(QObject *parent)
    : QObject(parent)
{
    setNumBands(4);
}

MultibandGate8::~MultibandGate8() = default;

/* ---- Configuration ---- */

void MultibandGate8::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
    // Redesign all crossovers
    for (int i = 0; i < m_crossovers.size(); ++i)
        designCrossover(m_crossoverFreqs[i], m_crossovers[i]);
    // Recompute envelope coefficients
    for (int b = 0; b < m_numBands; ++b) {
        double atk = m_bandParams[b].attack;
        double rel = m_bandParams[b].release;
        m_gateStates[b].coeffAttack = qExp(-1.0 / (m_sampleRate * atk * 0.001));
        m_gateStates[b].coeffRelease = qExp(-1.0 / (m_sampleRate * rel * 0.001));
    }
}

void MultibandGate8::setNumBands(int n)
{
    m_numBands = qBound(2, n, 16);
    int numXover = m_numBands - 1;
    m_crossovers.resize(numXover);
    m_crossoverFreqs.resize(numXover);
    m_bandParams.resize(m_numBands);
    m_gateStates.resize(m_numBands);
    m_bandOutputs.resize(m_numBands);

    // Default crossover frequencies: logarithmically spaced
    if (numXover > 0) {
        double fMin = 80.0, fMax = 12000.0;
        for (int i = 0; i < numXover; ++i) {
            double t = (double)(i + 1) / m_numBands;
            m_crossoverFreqs[i] = fMin * qPow(fMax / fMin, t);
            designCrossover(m_crossoverFreqs[i], m_crossovers[i]);
        }
    }

    for (int b = 0; b < m_numBands; ++b) {
        m_gateStates[b].gain = 1.0;
        m_gateStates[b].envLevel = -120.0;
        m_gateStates[b].gateOpen = false;
        m_gateStates[b].coeffAttack = qExp(-1.0 / (m_sampleRate * 0.001));
        m_gateStates[b].coeffRelease = qExp(-1.0 / (m_sampleRate * 0.050));
    }
}

void MultibandGate8::setCrossoverFreqs(const QVector<double>& freqs)
{
    int n = qMin(freqs.size(), m_crossovers.size());
    for (int i = 0; i < n; ++i) {
        m_crossoverFreqs[i] = qBound(20.0, freqs[i], m_sampleRate * 0.45);
        designCrossover(m_crossoverFreqs[i], m_crossovers[i]);
    }
}

void MultibandGate8::setBandParams(int band, const BandParams& params)
{
    if (band < 0 || band >= m_numBands) return;
    m_bandParams[band] = params;
    m_gateStates[band].coeffAttack = qExp(-1.0 / (m_sampleRate * params.attack * 0.001));
    m_gateStates[band].coeffRelease = qExp(-1.0 / (m_sampleRate * params.release * 0.001));
}

/* ---- dB conversion ---- */

double MultibandGate8::dbToLinear(double db) { return qPow(10.0, db * 0.05); }
double MultibandGate8::linearToDb(double lin) { return lin > 0.0 ? 20.0 * qLn(lin) / M_LN10 : -120.0; }

/* ---- Design 2nd-order Butterworth crossover ---- */

void MultibandGate8::designCrossover(double freq, CrossoverStage& stage)
{
    double omega = 2.0 * M_PI * freq / m_sampleRate;
    double cosW = qCos(omega);
    double sinW = qSin(omega);
    double Q = 0.7071067811865476; // Butterworth Q = 1/sqrt(2)
    double alpha = sinW / (2.0 * Q);

    double a0 = 1.0 + alpha;
    stage.b0 = (1.0 - cosW) / (2.0 * a0);
    stage.b1 = (1.0 - cosW) / a0;
    stage.b2 = (1.0 - cosW) / (2.0 * a0);
    stage.a1 = -2.0 * cosW / a0;
    stage.a2 = (1.0 - alpha) / a0;

    // HP coefficients (allpass complement for Linkwitz-Riley behavior)
    stage.lp = FilterState{};
    stage.hp = FilterState{};
}

/* ---- Filter one sample ---- */

double MultibandGate8::filterSample(double input, FilterState& fs,
                                     double b0, double b1, double b2,
                                     double a1, double a2) const
{
    double output = b0 * input + b1 * fs.x1 + b2 * fs.x2 - a1 * fs.y1 - a2 * fs.y2;
    fs.x2 = fs.x1; fs.x1 = input;
    fs.y2 = fs.y1; fs.y1 = output;
    return output;
}

/* ---- Update gate gain with hysteresis ---- */

void MultibandGate8::updateGate(int band, double level)
{
    GateState& gs = m_gateStates[band];
    const BandParams& bp = m_bandParams[band];

    if (bp.bypass) { gs.gain = 1.0; return; }

    double openThresh = bp.threshold;
    double closeThresh = bp.threshold - bp.hysteresis;

    // Envelope follower
    double coeff = (level > gs.envLevel) ? gs.coeffAttack : gs.coeffRelease;
    gs.envLevel = coeff * gs.envLevel + (1.0 - coeff) * level;

    // Hysteresis gate logic
    if (!gs.gateOpen && gs.envLevel >= openThresh)
        gs.gateOpen = true;
    else if (gs.gateOpen && gs.envLevel < closeThresh)
        gs.gateOpen = false;

    // Compute gain
    if (gs.gateOpen) {
        gs.gain = 1.0;
    } else {
        // Apply range (floor)
        double rangeGain = dbToLinear(bp.range);
        // Smooth transition: proportional to how far below threshold
        double distBelow = openThresh - gs.envLevel;
        double t = qBound(0.0, distBelow / qMax(bp.hysteresis, 1.0), 1.0);
        gs.gain = 1.0 + t * (rangeGain - 1.0);
    }
}

/* ---- Process a single sample ---- */

double MultibandGate8::processSample(double input)
{
    if (m_crossovers.isEmpty()) return input;

    // Split into bands via cascaded crossovers
    QVector<double> bands(m_numBands);
    double lpOut = input;

    for (int i = 0; i < m_crossovers.size(); ++i) {
        CrossoverStage& cs = m_crossovers[i];
        double hp = filterSample(lpOut, cs.hp,
                                 1.0 - cs.b0 * (cs.a0 > 0 ? 1 : 1),
                                 -(cs.b1), -(cs.b2), cs.a1, cs.a2);
        double lp = filterSample(lpOut, cs.lp, cs.b0, cs.b1, cs.b2, cs.a1, cs.a2);

        // HP output goes to this band (simplified)
        double bandSignal = lpOut - lp;
        bands[i] = bandSignal;

        lpOut = lp;
    }
    bands[m_numBands - 1] = lpOut; // Last band gets remaining LP

    // Apply gating per band
    double output = 0.0;
    for (int b = 0; b < m_numBands; ++b) {
        double level = linearToDb(qAbs(bands[b]));
        updateGate(b, level);
        m_bandOutputs[b] = bands[b] * m_gateStates[b].gain;
        output += m_bandOutputs[b];
    }

    m_stats.totalFrames++;
    return output;
}

/* ---- Process a block ---- */

QVector<double> MultibandGate8::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processSample(input[i]);

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit processingDone(input.size(), elapsed);
    return output;
}

/* ---- Get frame state ---- */

MultibandGate8::FrameResult MultibandGate8::getFrameState() const
{
    FrameResult fr;
    fr.numBands = m_numBands;
    fr.bandGains.resize(m_numBands);
    fr.bandLevels.resize(m_numBands);
    for (int b = 0; b < m_numBands; ++b) {
        fr.bandGains[b] = m_gateStates[b].gain;
        fr.bandLevels[b] = m_gateStates[b].envLevel;
    }
    return fr;
}

/* ---- Reset ---- */

void MultibandGate8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    for (int b = 0; b < m_numBands; ++b) {
        m_gateStates[b].gain = 1.0;
        m_gateStates[b].envLevel = -120.0;
        m_gateStates[b].gateOpen = false;
    }
    for (auto& cs : m_crossovers) {
        cs.lp = FilterState{};
        cs.hp = FilterState{};
    }
}
