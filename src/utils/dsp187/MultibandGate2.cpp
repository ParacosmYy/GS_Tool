/**
 * @file MultibandGate2.cpp
 * @brief MultibandGate2 实现
 *
 * 实现多频段门控：Linkwitz-Riley交叉、包络跟踪、噪声底估计、门控增益。
 */

#include "utils/dsp187/MultibandGate2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MultibandGate2::MultibandGate2(QObject *parent) : QObject(parent)
{
    initBands();
}

MultibandGate2::~MultibandGate2() = default;

/* ---- Configuration ---- */

void MultibandGate2::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); initBands(); }
void MultibandGate2::setNumBands(int bands) { m_numBands = qBound(2, bands, 8); initBands(); }

void MultibandGate2::setCrossoverFreqs(const QVector<double>& freqs)
{
    for (int i = 0; i < qMin(freqs.size(), m_bands.size()); ++i)
        m_bands[i].params.crossoverFreq = qBound(20.0, freqs[i], m_sampleRate / 2.0 - 100.0);
}

void MultibandGate2::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_bands.size())
        m_bands[band].params = params;
}

/* ---- Init bands ---- */

void MultibandGate2::initBands()
{
    m_bands.resize(m_numBands);
    // Default crossover frequencies (log-spaced)
    double fMin = 80.0, fMax = m_sampleRate / 2.0 - 1000.0;
    for (int i = 0; i < m_numBands; ++i) {
        if (i < m_numBands - 1) {
            double t = static_cast<double>(i + 1) / m_numBands;
            m_bands[i].params.crossoverFreq = fMin * qPow(fMax / fMin, t);
        } else {
            m_bands[i].params.crossoverFreq = fMax;
        }
        m_bands[i].params.threshold = -40.0 - i * 5.0;
        m_bands[i].params.attack = 5.0;
        m_bands[i].params.release = 50.0 + i * 10.0;
        m_bands[i].params.noiseFloor = -80.0;
        m_bands[i].envelope = 0.0;
        m_bands[i].noiseEstimate = -80.0;
        m_bands[i].gateGain = 1.0;
    }
}

/* ---- Butterworth 2nd-order coefficients ---- */

void MultibandGate2::butterworthCoeffs(double fc, double& a1, double& a2,
                                        double& b0, double& b1, double& b2) const
{
    double omega = 2.0 * M_PI * fc / m_sampleRate;
    double cosW = qCos(omega);
    double sinW = qSin(omega);
    double alpha = sinW / (2.0 * 0.7071); // Q = 0.7071 for Butterworth
    double norm = 1.0 + alpha;
    a1 = -2.0 * cosW / norm;
    a2 = (1.0 - alpha) / norm;
    b0 = (1.0 - cosW) / 2.0 / norm;
    b1 = (1.0 - cosW) / norm;
    b2 = b0;
}

/* ---- Process 2nd-order LP ---- */

double MultibandGate2::processLP(double x, LRState& s,
                                  double a1, double a2, double b0, double b1, double b2)
{
    double y = b0 * x + b1 * s.x1 + b2 * s.x2 - a1 * s.y1 - a2 * s.y2;
    s.x2 = s.x1; s.x1 = x;
    s.y2 = s.y1; s.y1 = y;
    return y;
}

/* ---- Process 2nd-order HP ---- */

double MultibandGate2::processHP(double x, LRState& s,
                                  double a1, double a2, double b0, double b1, double b2)
{
    // HP = x - LP output (complementary)
    double y = b0 * x + b1 * s.x1 + b2 * s.x2 - a1 * s.y1 - a2 * s.y2;
    s.x2 = s.x1; s.x1 = x;
    s.y2 = s.y1; s.y1 = y;
    return y;
}

/* ---- Crossover split (Linkwitz-Riley 4th-order = 2x 2nd-order cascade) ---- */

void MultibandGate2::crossoverSplit(const QVector<double>& input,
                                     QVector<QVector<double>>& bandSignals)
{
    int N = input.size();
    int numFilters = m_bands.size() - 1;
    bandSignals.resize(m_bands.size());

    for (int b = 0; b < m_bands.size(); ++b)
        bandSignals[b].resize(N);

    // Progressive band splitting
    QVector<double> remaining = input;
    for (int f = 0; f < numFilters; ++f) {
        double fc = m_bands[f].params.crossoverFreq;
        double a1, a2, b0, b1, b2;
        butterworthCoeffs(fc, a1, a2, b0, b1, b2);

        QVector<double> lp(N), hp(N);
        for (int i = 0; i < N; ++i) {
            double lpOut = processLP(remaining[i], m_bands[f].lpState1, a1, a2, b0, b1, b2);
            lpOut = processLP(lpOut, m_bands[f].lpState2, a1, a2, b0, b1, b2);
            lp[i] = lpOut;

            // HP coefficients (complementary)
            double hpb0 = (1.0 + a2) / 2.0;
            double hpb1 = -(1.0 + a1 + a2) / 2.0;  // simplified
            double hpb2 = 0.0;
            double hpa1 = a1, hpa2 = a2;
            // Use standard HP coeff computation
            double hpOut = remaining[i] - lpOut; // Simplified complementary
            hp[i] = hpOut;
        }
        bandSignals[f] = lp;
        remaining = hp;
    }
    bandSignals[numFilters] = remaining;
}

/* ---- Envelope follower ---- */

double MultibandGate2::followEnvelope(double sample, const BandState& band)
{
    double absVal = qFabs(sample);
    double coeff = (absVal > band.envelope)
        ? 1.0 - qExp(-1.0 / (m_sampleRate * band.params.attack * 0.001))
        : 1.0 - qExp(-1.0 / (m_sampleRate * band.params.release * 0.001));
    return band.envelope + coeff * (absVal - band.envelope);
}

/* ---- Noise floor tracking ---- */

double MultibandGate2::trackNoiseFloor(double envelope, double currentEstimate)
{
    // Slowly decaying estimate
    double decayRate = 0.9999;
    double newEstimate = currentEstimate * decayRate;
    if (envelope < currentEstimate * 1.5) {
        // Likely noise, update estimate upward
        newEstimate = qMax(newEstimate, envelope * 0.95);
    }
    return newEstimate;
}

/* ---- Compute gate gain ---- */

double MultibandGate2::computeGateGain(double envelope, double threshold,
                                        double noiseFloor) const
{
    // Convert to dB
    double envDb = 20.0 * qLog10(qMax(envelope, 1e-10));
    double threshDb = threshold;
    double noiseDb = noiseFloor;

    // Gate opens when envelope exceeds threshold + hysteresis above noise
    double hysteresis = 6.0; // dB
    if (envDb > threshDb && envDb > noiseDb + hysteresis)
        return 1.0; // Fully open
    if (envDb < threshDb - 10.0 || envDb < noiseDb)
        return 0.0; // Fully closed
    // Smooth transition
    double range = 10.0;
    double t = (envDb - (threshDb - range)) / range;
    return qBound(0.0, t, 1.0);
}

/* ---- Main process ---- */

QVector<double> MultibandGate2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {};

    // Split into bands
    QVector<QVector<double>> bandSignals;
    crossoverSplit(input, bandSignals);

    // Process each band
    QVector<double> output(N, 0.0);
    for (int b = 0; b < m_bands.size(); ++b) {
        for (int i = 0; i < N; ++i) {
            m_bands[b].envelope = followEnvelope(bandSignals[b][i], m_bands[b]);
            m_bands[b].noiseEstimate = trackNoiseFloor(m_bands[b].envelope,
                                                        m_bands[b].noiseEstimate);
            double gain = computeGateGain(m_bands[b].envelope,
                                          m_bands[b].params.threshold,
                                          m_bands[b].params.noiseFloor);
            m_bands[b].gateGain = gain;
            m_bands[b].params.reduction = 20.0 * qLog10(qMax(gain, 1e-10));
            output[i] += bandSignals[b][i] * gain;
        }
    }

    m_stats.totalProcessCalls++;
    m_stats.blockSize = N;
    m_stats.numBands = m_bands.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(N, timer.elapsed());
    return output;
}

/* ---- Band gains ---- */

QVector<double> MultibandGate2::bandGains() const
{
    QVector<double> gains;
    for (const auto& b : m_bands)
        gains.append(b.params.reduction);
    return gains;
}

/* ---- Noise floor estimates ---- */

QVector<double> MultibandGate2::noiseFloorEstimates() const
{
    QVector<double> nf;
    for (const auto& b : m_bands)
        nf.append(20.0 * qLog10(qMax(b.noiseEstimate, 1e-10)));
    return nf;
}

/* ---- Reset ---- */

void MultibandGate2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    initBands();
}
