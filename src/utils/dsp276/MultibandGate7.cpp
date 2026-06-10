/**
 * @file MultibandGate7.cpp
 * @brief MultibandGate7 实现
 *
 * 实现多频段门限：逐频段动态压缩与瞬态检测的多通道动态控制。
 */

#include "utils/dsp276/MultibandGate7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandGate7::MultibandGate7(QObject *parent)
    : QObject(parent)
{
    setNumBands(4);
}

MultibandGate7::~MultibandGate7() = default;

/* ---- Configuration ---- */

void MultibandGate7::setSampleRate(double rate)
{
    m_sampleRate = qBound(8000.0, rate, 192000.0);
    initFilters();
}

void MultibandGate7::setNumBands(int n)
{
    m_numBands = qBound(1, n, 16);
    m_bandConfigs.resize(m_numBands);
    m_envelope.resize(m_numBands, 0.0);
    m_gain.resize(m_numBands, 1.0);
    m_transient.resize(m_numBands, false);
    m_prevEnvelope.resize(m_numBands, 0.0);

    // Default crossover distribution (logarithmic)
    for (int i = 0; i < m_numBands; ++i) {
        double f = 60.0 * qPow(1000.0 / 60.0, static_cast<double>(i) / m_numBands);
        m_bandConfigs[i].lowFreq = (i == 0) ? 0.0 : f;
        m_bandConfigs[i].highFreq = (i == m_numBands - 1) ? m_sampleRate / 2.0 : f;
        if (i > 0) m_bandConfigs[i - 1].highFreq = f;
    }
    initFilters();
}

void MultibandGate7::setBandConfig(int band, const BandConfig& config)
{
    if (band >= 0 && band < m_numBands) m_bandConfigs[band] = config;
}

/* ---- Design Linkwitz-Riley 4th-order crossover ---- */

void MultibandGate7::designCrossover(LRFilter& lp, LRFilter& hp, double freq)
{
    // Butterworth 2nd-order coefficients (cascaded twice for LR4)
    double wc = 2.0 * M_PI * freq / m_sampleRate;
    double wc2 = wc * wc;
    double sqrt2 = qSqrt(2.0);

    for (int s = 0; s < 2; ++s) {
        // Lowpass (Butterworth 2nd order)
        double norm = 1.0 + sqrt2 * wc + wc2;
        lp.b0[s] = wc2 / norm;
        lp.b1[s] = 2.0 * wc2 / norm;
        lp.b2[s] = wc2 / norm;
        lp.a1[s] = 2.0 * (wc2 - 1.0) / norm;
        lp.a2[s] = (1.0 - sqrt2 * wc + wc2) / norm;

        // Highpass (Butterworth 2nd order)
        hp.b0[s] = 1.0 / norm;
        hp.b1[s] = -2.0 / norm;
        hp.b2[s] = 1.0 / norm;
        hp.a1[s] = lp.a1[s];  // Same denominator
        hp.a2[s] = lp.a2[s];
    }
}

/* ---- Process one biquad section ---- */

double MultibandGate7::processBiquad(double in, LRFilter& f, int section)
{
    double out = f.b0[section] * in + f.b1[section] * f.x1[section]
                 + f.b2[section] * f.x2[section]
                 - f.a1[section] * f.y1[section]
                 - f.a2[section] * f.y2[section];
    f.x2[section] = f.x1[section];
    f.x1[section] = in;
    f.y2[section] = f.y1[section];
    f.y1[section] = out;
    return out;
}

/* ---- Initialize crossover filters ---- */

void MultibandGate7::initFilters()
{
    int numCrossovers = qMax(0, m_numBands - 1);
    m_lowpass.resize(numCrossovers);
    m_highpass.resize(numCrossovers);

    for (int i = 0; i < numCrossovers; ++i) {
        double freq = m_bandConfigs[i].highFreq;
        if (freq <= 0.0) freq = 1000.0;
        designCrossover(m_lowpass[i], m_highpass[i], freq);
    }
}

/* ---- Compute gate gain from envelope ---- */

double MultibandGate7::computeGateGain(double envelope, const BandConfig& cfg) const
{
    if (!cfg.enabled) return 1.0;

    double envDb = (envelope > 1e-10) ? 20.0 * qLn(envelope) / M_LN10 : -120.0;

    if (envDb >= cfg.threshold) {
        return qPow(10.0, cfg.makeupGain / 20.0);  // Above threshold: no gating
    }

    // Below threshold: apply gate with ratio
    double diffDb = cfg.threshold - envDb;
    double gateDb = -diffDb * (cfg.ratio - 1.0) / cfg.ratio;

    // Clamp gate depth to -80 dB
    gateDb = qMax(gateDb, -80.0);
    return qPow(10.0, (gateDb + cfg.makeupGain) / 20.0);
}

/* ---- Transient detection ---- */

bool MultibandGate7::detectTransient(double current, double previous, double threshold) const
{
    if (previous < 1e-10) return current > threshold;
    double riseDb = 20.0 * qLn(current / qMax(1e-10, previous)) / M_LN10;
    return riseDb > 12.0;  // 12 dB rise = transient
}

/* ---- Split signal into frequency bands ---- */

QVector<QVector<double>>
MultibandGate7::splitBands(const QVector<double>& input, int channels)
{
    int n = input.size();
    QVector<QVector<double>> bands(m_numBands);
    for (auto& b : bands) b.resize(n);

    if (m_numBands == 1) {
        bands[0] = input;
        return bands;
    }

    // Sequential crossover: split input into low/high, then recurse
    QVector<double> current = input;
    for (int i = 0; i < m_numBands - 1; ++i) {
        bands[i].resize(n);
        QVector<double> high(n);
        for (int s = 0; s < n; ++s) {
            double sample = current[s];
            // Process through 2 biquad sections for LR4
            double lp = processBiquad(sample, m_lowpass[i], 0);
            lp = processBiquad(lp, m_lowpass[i], 1);
            double hp = processBiquad(sample, m_highpass[i], 0);
            hp = processBiquad(hp, m_highpass[i], 1);
            bands[i][s] = lp;
            high[s] = hp;
        }
        current = high;
    }
    bands[m_numBands - 1] = current;
    return bands;
}

/* ---- Process block ---- */

QVector<double> MultibandGate7::process(const QVector<double>& input, int channels)
{
    QElapsedTimer timer;
    timer.start();

    m_channels = qBound(1, channels, 8);
    int n = input.size();
    if (n == 0) return {};

    double peakReduction = 0.0;
    QVector<double> output(n, 0.0);

    // Split into bands
    auto bands = splitBands(input, m_channels);

    // Process each band
    for (int b = 0; b < m_numBands; ++b) {
        const BandConfig& cfg = m_bandConfigs[b];
        double coeffA = qExp(-1.0 / (m_sampleRate * cfg.attack * 0.001));
        double coeffR = qExp(-1.0 / (m_sampleRate * cfg.release * 0.001));

        double peakBefore = 0.0;
        double peakAfter = 0.0;

        for (int s = 0; s < n; ++s) {
            double absVal = qAbs(bands[b][s]);

            // Envelope follower
            double coeff = (absVal > m_envelope[b]) ? coeffA : coeffR;
            m_prevEnvelope[b] = m_envelope[b];
            m_envelope[b] = coeff * m_envelope[b] + (1.0 - coeff) * absVal;

            // Transient detection
            m_transient[b] = detectTransient(m_envelope[b], m_prevEnvelope[b], 0.01);

            // Compute gate gain
            m_gain[b] = computeGateGain(m_envelope[b], cfg);

            // Apply gain
            double gated = bands[b][s] * m_gain[b];
            output[s] += gated;

            peakBefore = qMax(peakBefore, absVal);
            peakAfter = qMax(peakAfter, qAbs(gated));
        }

        double beforeDb = (peakBefore > 1e-10) ? 20.0 * qLn(peakBefore) / M_LN10 : -120.0;
        double afterDb = (peakAfter > 1e-10) ? 20.0 * qLn(peakAfter) / M_LN10 : -120.0;
        peakReduction = qMax(peakReduction, beforeDb - afterDb);
    }

    double elapsed = timer.elapsed();
    m_stats.numBands = m_numBands;
    m_stats.blockSize = n;
    m_stats.peakReductionDb = peakReduction;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(m_numBands, n, peakReduction, elapsed);

    return output;
}

/* ---- Get per-band gains ---- */

QVector<double> MultibandGate7::bandGains() const { return m_gain; }

/* ---- Get transient flags ---- */

QVector<bool> MultibandGate7::transients() const { return m_transient; }

/* ---- Reset internal state ---- */

void MultibandGate7::reset()
{
    m_envelope.fill(0.0);
    m_gain.fill(1.0);
    m_transient.fill(false);
    m_prevEnvelope.fill(0.0);
    for (auto& f : m_lowpass) {
        for (int s = 0; s < 2; ++s) { f.x1[s] = f.x2[s] = f.y1[s] = f.y2[s] = 0.0; }
    }
    for (auto& f : m_highpass) {
        for (int s = 0; s < 2; ++s) { f.x1[s] = f.x2[s] = f.y1[s] = f.y2[s] = 0.0; }
    }
}

/* ---- Reset ---- */

void MultibandGate7::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
