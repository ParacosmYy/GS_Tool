/**
 * @file Compressor7.cpp
 * @brief Compressor7 实现
 *
 * 实现多频段联动压缩器：动态EQ模式与频率依赖压缩比。
 */

#include "utils/dsp257/Compressor7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Compressor7::Compressor7(QObject *parent)
    : QObject(parent) { initFilters(); }
Compressor7::~Compressor7() = default;

/* ---- Configuration ---- */

void Compressor7::setSampleRate(int rate) { m_sampleRate = qMax(8000, rate); initFilters(); }
void Compressor7::setBands(int num) { m_numBands = qMax(1, num); initFilters(); }
void Compressor7::setBandConfig(int idx, const BandConfig& config)
{
    if (idx >= 0 && idx < m_configs.size())
        m_configs[idx] = config;
}
void Compressor7::setDynamicEQMode(bool enabled) { m_dynamicEQ = enabled; }
void Compressor7::setLinkageStrength(double strength) { m_linkage = qBound(0.0, strength, 1.0); }

/* ---- Initialize band filter states ---- */

void Compressor7::initFilters()
{
    m_configs.resize(m_numBands);
    m_states.resize(m_numBands);
    m_gainReductions.resize(m_numBands, 0.0);

    // Default: logarithmically spaced bands
    double nyquist = m_sampleRate / 2.0;
    for (int i = 0; i < m_numBands; ++i) {
        double fLow = (i == 0) ? 20.0 : nyquist * qPow(10.0, -1.0 + i * 1.0 / m_numBands);
        double fHigh = (i == m_numBands - 1) ? nyquist : nyquist * qPow(10.0, -1.0 + (i + 1) * 1.0 / m_numBands);
        m_configs[i].freqLow = fLow;
        m_configs[i].freqHigh = fHigh;
        m_configs[i].threshold = -20.0;
        m_configs[i].ratio = 4.0;
        m_configs[i].attack = 10.0;
        m_configs[i].release = 100.0;
        m_configs[i].knee = 6.0;
        m_configs[i].makeupGain = 0.0;
        m_configs[i].bypass = false;

        // Initialize 2nd-order state-variable filter state
        m_states[i].filterX1.resize(2, 0.0);
        m_states[i].filterX2.resize(2, 0.0);
        m_states[i].filterY1.resize(2, 0.0);
        m_states[i].filterY2.resize(2, 0.0);
        m_states[i].envelope = -120.0;
        m_states[i].gainReduction = 0.0;
    }
}

/* ---- Design crossover filter for band ---- */

void Compressor7::designCrossover(int band, double freqLow, double freqHigh)
{
    Q_UNUSED(band)
    Q_UNUSED(freqLow)
    Q_UNUSED(freqHigh)
    // Filters are implicit via state-variable filter coefficients
}

/* ---- Detect envelope for band ---- */

double Compressor7::detectEnvelope(int band, double sample)
{
    BandConfig& cfg = m_configs[band];
    BandState& state = m_states[band];

    // Simple envelope follower with attack/release
    double absSample = qFabs(sample);
    double alphaAttack = qExp(-1.0 / (m_sampleRate * cfg.attack / 1000.0));
    double alphaRelease = qExp(-1.0 / (m_sampleRate * cfg.release / 1000.0));

    double env = state.envelope;
    if (absSample > env)
        env = alphaAttack * env + (1.0 - alphaAttack) * absSample;
    else
        env = alphaRelease * env + (1.0 - alphaRelease) * absSample;

    state.envelope = env;
    // Convert to dB
    return 20.0 * qLn(qMax(env, 1e-10)) / M_LN2 / 6.0;  // approx dB
}

/* ---- Compute gain reduction using soft knee ---- */

double Compressor7::computeGainReduction(double levelDB, const BandConfig& cfg) const
{
    if (levelDB < cfg.threshold - cfg.knee / 2.0)
        return 0.0;

    double ratio = cfg.ratio;
    // Frequency-dependent compression ratio in dynamic EQ mode
    if (m_dynamicEQ) {
        double centerFreq = (cfg.freqLow + cfg.freqHigh) / 2.0;
        double midFreq = m_sampleRate / 4.0;
        ratio = cfg.ratio * (1.0 + 0.5 * centerFreq / midFreq);
    }

    if (levelDB < cfg.threshold + cfg.knee / 2.0) {
        // Soft knee region: quadratic interpolation
        double x = levelDB - (cfg.threshold - cfg.knee / 2.0);
        double halfKnee = cfg.knee / 2.0;
        return (x * x) / (2.0 * cfg.knee) * (ratio - 1.0) / ratio;
    }
    // Above knee: full compression
    return (levelDB - cfg.threshold) * (1.0 - 1.0 / ratio);
}

/* ---- Apply linkage across bands ---- */

void Compressor7::applyLinkage()
{
    if (m_linkage <= 0.0) return;

    // Compute average gain reduction
    double avgGR = 0.0;
    for (int i = 0; i < m_numBands; ++i)
        avgGR += m_gainReductions[i];
    avgGR /= m_numBands;

    // Blend individual and average
    for (int i = 0; i < m_numBands; ++i)
        m_gainReductions[i] = (1.0 - m_linkage) * m_gainReductions[i] + m_linkage * avgGR;
}

/* ---- Process block ---- */

QVector<double> Compressor7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    double inPeak = 0.0, outPeak = 0.0;

    for (int s = 0; s < n; ++s) {
        double sample = input[s];
        double processed = 0.0;
        inPeak = qMax(inPeak, qFabs(sample));

        for (int b = 0; b < m_numBands; ++b) {
            if (m_configs[b].bypass) continue;

            // Simple bandpass via state-variable filter (2nd order approximation)
            BandState& st = m_states[b];
            double fc = (m_configs[b].freqLow + m_configs[b].freqHigh) / 2.0;
            double freq = 2.0 * qSin(M_PI * fc / m_sampleRate);
            st.filterY1[0] = st.filterY1[0] + freq * st.filterX2[0];
            st.filterX2[0] = st.filterX2[0] - freq * st.filterY1[0];
            double bandSample = st.filterY1[0] * sample;

            // Detect envelope
            double levelDB = detectEnvelope(b, bandSample);

            // Compute gain reduction
            double gr = computeGainReduction(levelDB, m_configs[b]);
            m_gainReductions[b] = gr;
            st.gainReduction = gr;

            // Apply gain in linear domain
            double gainLin = qPow(10.0, (-gr + m_configs[b].makeupGain) / 20.0);
            processed += bandSample * gainLin;
        }

        output[s] = processed;
        outPeak = qMax(outPeak, qFabs(processed));
    }

    // Apply inter-band linkage
    applyLinkage();

    double elapsed = timer.elapsed();
    m_stats.sampleRate = m_sampleRate;
    m_stats.numBands = m_numBands;
    m_stats.inputLevel = inPeak;
    m_stats.outputLevel = outPeak;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit compressionCompleted(inPeak, outPeak, elapsed);
    return output;
}

/* ---- Get gain reductions ---- */

QVector<double> Compressor7::gainReductions() const { return m_gainReductions; }

/* ---- Reset ---- */

void Compressor7::resetStatistics()
{
    initFilters();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
