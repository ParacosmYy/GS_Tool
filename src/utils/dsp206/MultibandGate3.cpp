/**
 * @file MultibandGate3.cpp
 * @brief MultibandGate3 实现
 *
 * 实现多频段噪声门：Bark频段分裂、前瞻增益控制、频段合并。
 */

#include "utils/dsp206/MultibandGate3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MultibandGate3::MultibandGate3(QObject *parent) : QObject(parent)
{
    m_bandStates.resize(m_numBands);
    for (int i = 0; i < m_numBands; ++i) {
        m_bandStates[i].threshold = -40.0 + i * 3.0; // Higher bands less aggressive
        m_bandStates[i].attack = 1.0;
        m_bandStates[i].release = 50.0 + i * 10.0;
    }
}

MultibandGate3::~MultibandGate3() = default;

/* ---- Configuration ---- */

void MultibandGate3::setSampleRate(int rate) { m_sampleRate = qMax(1, rate); }
void MultibandGate3::setBlockSize(int size) { m_blockSize = qMax(1, size); }

void MultibandGate3::setNumBands(int n)
{
    m_numBands = qMax(1, qMin(n, 24));
    m_bandStates.resize(m_numBands);
}

/* ---- Hz <-> Bark conversion ---- */

double MultibandGate3::hzToBark(double hz)
{
    return 13.0 * qAtan(0.00076 * hz) + 3.5 * qAtan((hz / 7500.0) * (hz / 7500.0));
}

double MultibandGate3::barkToHz(double bark)
{
    // Inverse approximation via bisection
    double lo = 0.0, hi = 20000.0;
    for (int i = 0; i < 30; ++i) {
        double mid = (lo + hi) * 0.5;
        if (hzToBark(mid) < bark) lo = mid;
        else hi = mid;
    }
    return (lo + hi) * 0.5;
}

/* ---- Design crossover frequencies ---- */

QVector<double> MultibandGate3::designCrossoverFreqs() const
{
    double maxBark = hzToBark(m_sampleRate * 0.5);
    QVector<double> freqs(m_numBands + 1);
    for (int i = 0; i <= m_numBands; ++i) {
        double bark = (maxBark * i) / m_numBands;
        freqs[i] = barkToHz(bark);
    }
    return freqs;
}

/* ---- Simple low-pass filter ---- */

QVector<double> MultibandGate3::lowpass(const QVector<double>& in, double cutoff, int sr)
{
    double rc = 1.0 / (2.0 * M_PI * qMax(cutoff, 1.0));
    double dt = 1.0 / sr;
    double alpha = dt / (rc + dt);
    QVector<double> out(in.size(), 0.0);
    if (in.isEmpty()) return out;
    out[0] = in[0];
    for (int i = 1; i < in.size(); ++i)
        out[i] = out[i - 1] + alpha * (in[i] - out[i - 1]);
    return out;
}

/* ---- Simple high-pass filter ---- */

QVector<double> MultibandGate3::highpass(const QVector<double>& in, double cutoff, int sr)
{
    double rc = 1.0 / (2.0 * M_PI * qMax(cutoff, 1.0));
    double dt = 1.0 / sr;
    double alpha = rc / (rc + dt);
    QVector<double> out(in.size(), 0.0);
    if (in.isEmpty()) return out;
    out[0] = in[0];
    for (int i = 1; i < in.size(); ++i)
        out[i] = alpha * (out[i - 1] + in[i] - in[i - 1]);
    return out;
}

/* ---- Split into Bark bands ---- */

QVector<QVector<double>> MultibandGate3::splitBands(const QVector<double>& input) const
{
    QVector<double> freqs = designCrossoverFreqs();
    QVector<QVector<double>> bands(m_numBands);

    // Cumulative filter approach: extract each band
    QVector<double> remaining = input;
    for (int b = 0; b < m_numBands - 1; ++b) {
        double cutoff = freqs[b + 1];
        bands[b] = lowpass(remaining, cutoff, m_sampleRate);
        remaining = highpass(remaining, cutoff, m_sampleRate);
    }
    bands[m_numBands - 1] = remaining;
    return bands;
}

/* ---- Band level in dB ---- */

double MultibandGate3::bandLevelDb(const QVector<double>& band) const
{
    double sumSq = 0.0;
    for (double s : band) sumSq += s * s;
    double rms = qSqrt(sumSq / qMax(1, band.size()));
    if (rms < 1e-10) return -120.0;
    return 20.0 * qLn(rms) / qLn(10.0);
}

/* ---- Apply lookahead gate ---- */

QVector<double> MultibandGate3::applyLookaheadGate(const QVector<double>& band, BandState& state) const
{
    int n = band.size();
    if (n == 0) return band;

    int sr = m_sampleRate;
    int attackSamples = qMax(1, static_cast<int>(state.attack * sr / 1000.0));
    int releaseSamples = qMax(1, static_cast<int>(state.release * sr / 1000.0));

    // Lookahead buffer: compute envelope from future samples
    QVector<double> gain(n, 0.0);
    for (int i = 0; i < n; ++i) {
        // Compute local RMS over lookahead window
        double sumSq = 0.0;
        int winLen = qMin(attackSamples, n - i);
        for (int j = i; j < i + winLen; ++j)
            sumSq += band[j] * band[j];
        double rmsDb = (sumSq > 0.0) ? 20.0 * qLn(qSqrt(sumSq / winLen)) / qLn(10.0) : -120.0;

        // Smooth envelope
        double coeff = (rmsDb > state.envelope) ? 1.0 / attackSamples : 1.0 / releaseSamples;
        state.envelope += coeff * (rmsDb - state.envelope);

        // Compute gain reduction
        if (state.envelope < state.threshold) {
            state.reduction = state.threshold - state.envelope;
            state.gated = true;
        } else {
            state.reduction = qMax(0.0, state.reduction - 1.0 / releaseSamples * 60.0);
            state.gated = false;
        }

        double gainLin = qPow(10.0, -state.reduction / 20.0);
        gain[i] = gainLin;
    }

    QVector<double> out(n);
    for (int i = 0; i < n; ++i)
        out[i] = band[i] * gain[i];
    return out;
}

/* ---- Merge bands ---- */

QVector<double> MultibandGate3::mergeBands(const QVector<QVector<double>>& bands) const
{
    if (bands.isEmpty()) return {};
    int n = bands[0].size();
    QVector<double> out(n, 0.0);
    for (const auto& band : bands)
        for (int i = 0; i < qMin(n, band.size()); ++i)
            out[i] += band[i];
    return out;
}

/* ---- Process block ---- */

QVector<double> MultibandGate3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> bands = splitBands(input);
    for (int b = 0; b < m_numBands; ++b)
        bands[b] = applyLookaheadGate(bands[b], m_bandStates[b]);
    QVector<double> output = mergeBands(bands);

    m_stats.totalBlocks++;
    m_stats.numBands = m_numBands;
    m_stats.blockSize = input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;

    emit processingCompleted(m_numBands, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void MultibandGate3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    for (auto& bs : m_bandStates) {
        bs.reduction = 0.0;
        bs.envelope = 0.0;
        bs.gated = false;
    }
}
