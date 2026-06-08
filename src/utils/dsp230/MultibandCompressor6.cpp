/**
 * @file MultibandCompressor6.cpp
 * @brief MultibandCompressor6 实现
 *
 * 实现多频段压缩器：FIR交叉分频、前瞻限制与频段间增益共享。
 */

#include "utils/dsp230/MultibandCompressor6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandCompressor6::MultibandCompressor6(QObject *parent) : QObject(parent) {}
MultibandCompressor6::~MultibandCompressor6() = default;

/* ---- Configure ---- */

bool MultibandCompressor6::configure(int numBands, const QVector<double>& crossoverFreqs,
                                       int firTaps, double sampleRate)
{
    if (numBands < 2 || numBands > 8) return false;
    if (crossoverFreqs.size() != numBands - 1) return false;
    if (firTaps < 4 || sampleRate <= 0) return false;

    m_numBands = numBands;
    m_firTaps = firTaps;
    m_sampleRate = sampleRate;
    m_lookaheadSamples = qMin(128, firTaps);

    m_bandConfigs.resize(numBands);
    m_crossoverCoeffs.resize(numBands - 1);
    m_filterStates.resize(numBands - 1);
    m_envelopeState.resize(numBands, -60.0);
    m_gainReduction.resize(numBands, 0.0);

    // Design FIR crossover filters for each crossover frequency
    for (int i = 0; i < crossoverFreqs.size(); ++i) {
        m_crossoverCoeffs[i] = designCrossoverFIR(crossoverFreqs[i], firTaps);
        m_filterStates[i].resize(firTaps, 0.0);
    }

    m_lookaheadBuffer.resize(m_lookaheadSamples, 0.0);
    m_lookaheadPos = 0;

    m_stats.numBands = numBands;
    m_stats.numTaps = firTaps;
    return true;
}

/* ---- Design FIR crossover filter ---- */

QVector<double> MultibandCompressor6::designCrossoverFIR(double cutoffFreq, int taps) const
{
    QVector<double> coeffs(taps);
    int mid = taps / 2;
    double omega = 2.0 * cutoffFreq / m_sampleRate;

    for (int n = 0; n < taps; ++n) {
        // Windowed sinc low-pass filter
        int k = n - mid;
        double sinc = (k == 0) ? omega : qSin(M_PI * omega * k) / (M_PI * k);
        // Blackman window
        double w = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (taps - 1))
                   + 0.08 * qCos(4.0 * M_PI * n / (taps - 1));
        coeffs[n] = sinc * w;
    }
    return coeffs;
}

/* ---- Apply FIR filter ---- */

double MultibandCompressor6::applyFIR(const QVector<double>& coeffs,
                                        QVector<double>& state, double sample) const
{
    // Shift state and insert new sample
    for (int i = state.size() - 1; i > 0; --i)
        state[i] = state[i - 1];
    state[0] = sample;

    double output = 0.0;
    for (int i = 0; i < coeffs.size(); ++i)
        output += coeffs[i] * state[i];
    return output;
}

/* ---- Compute gain reduction ---- */

double MultibandCompressor6::computeGainReduction(double levelDb,
                                                    const BandConfig& cfg) const
{
    if (cfg.bypass || levelDb <= cfg.threshold) return 0.0;
    double overDb = levelDb - cfg.threshold;
    return -overDb * (1.0 - 1.0 / cfg.ratio);
}

/* ---- Envelope follower ---- */

double MultibandCompressor6::envelopeFollow(double current, double previous,
                                              double attackCoeff,
                                              double releaseCoeff) const
{
    if (current > previous)
        return previous + attackCoeff * (current - previous);
    else
        return previous + releaseCoeff * (current - previous);
}

/* ---- Inter-band gain sharing ---- */

double MultibandCompressor6::sharedGainAdjustment(const QVector<double>& reductions) const
{
    // Find the maximum gain reduction across all bands
    double maxRed = 0.0;
    for (double r : reductions)
        maxRed = qMin(maxRed, r);

    // Share a fraction of the strongest reduction with other bands
    return maxRed * 0.3;
}

/* ---- Set band config ---- */

void MultibandCompressor6::setBandConfig(int band, const BandConfig& config)
{
    if (band >= 0 && band < m_bandConfigs.size())
        m_bandConfigs[band] = config;
}

/* ---- Process block ---- */

QVector<double> MultibandCompressor6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int s = 0; s < n; ++s) {
        double sample = input[s];

        // Split into bands using cascaded crossover filters
        QVector<double> bandSignals(m_numBands);
        bandSignals[0] = sample;

        for (int b = 0; b < m_numBands - 1; ++b) {
            double low = applyFIR(m_crossoverCoeffs[b], m_filterStates[b], bandSignals[b]);
            double high = bandSignals[b] - low;
            bandSignals[b] = low;
            bandSignals[b + 1] = high;
        }

        // Apply per-band compression
        for (int b = 0; b < m_numBands; ++b) {
            double level = qMax(-120.0, 20.0 * qLog10(qMax(1e-10, qAbs(bandSignals[b]))));
            double attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_bandConfigs[b].attack / 1000.0));
            double releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * m_bandConfigs[b].release / 1000.0));

            m_envelopeState[b] = envelopeFollow(level, m_envelopeState[b],
                                                  attackCoeff, releaseCoeff);
            m_gainReduction[b] = computeGainReduction(m_envelopeState[b], m_bandConfigs[b]);
        }

        // Inter-band gain sharing
        double shared = sharedGainAdjustment(m_gainReduction);

        // Apply gain reduction + lookahead delay
        m_lookaheadBuffer[m_lookaheadPos] = sample;
        int delayedPos = (m_lookaheadPos + m_lookaheadBuffer.size() - m_lookaheadSamples)
                         % m_lookaheadBuffer.size();
        double delayed = m_lookaheadBuffer[delayedPos];
        m_lookaheadPos = (m_lookaheadPos + 1) % m_lookaheadBuffer.size();

        // Reconstruct output
        double outSample = 0.0;
        for (int b = 0; b < m_numBands; ++b) {
            double gain = qPow(10.0, (m_gainReduction[b] + shared) / 20.0);
            gain *= qPow(10.0, m_bandConfigs[b].makeupGain / 20.0);
            outSample += bandSignals[b] * gain;
        }
        output[s] = outSample;
    }

    m_stats.blockSize = n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, timer.elapsed());
    emit gainReductionChanged(m_gainReduction);
    return output;
}

/* ---- Get gain reduction ---- */

QVector<double> MultibandCompressor6::gainReduction() const
{
    return m_gainReduction;
}

/* ---- Reset ---- */

void MultibandCompressor6::reset()
{
    for (auto& state : m_filterStates)
        state.fill(0.0);
    m_envelopeState.fill(-60.0);
    m_gainReduction.fill(0.0);
    m_lookaheadBuffer.fill(0.0);
    m_lookaheadPos = 0;
    m_sharedGain = 0.0;
}

/* ---- Reset statistics ---- */

void MultibandCompressor6::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
