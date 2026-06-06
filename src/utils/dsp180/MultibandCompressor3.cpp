/**
 * @file MultibandCompressor3.cpp
 * @brief MultibandCompressor3 实现
 *
 * 实现多频段压缩器：Linkwitz-Riley 4阶分频、每带压缩/扩展、Mid/Side处理。
 */

#include "utils/dsp180/MultibandCompressor3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandCompressor3::MultibandCompressor3(QObject *parent) : QObject(parent)
{
    m_crossoverFreqs = {80.0, 500.0, 4000.0};
    m_bandParams.resize(4);
    m_filterState.resize(3);
    m_envLevel.resize(4, 0.0);
    m_gainReduction.resize(4, 0.0);
}

MultibandCompressor3::~MultibandCompressor3() = default;

/* ---- Configuration ---- */

void MultibandCompressor3::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); }
void MultibandCompressor3::setNumBands(int bands) { m_numBands = qMax(2, bands); }
void MultibandCompressor3::setCrossoverFreqs(const QVector<double>& freqs)
{
    m_crossoverFreqs = freqs;
    m_filterState.resize(freqs.size());
    m_numBands = freqs.size() + 1;
    m_bandParams.resize(m_numBands);
    m_envLevel.resize(m_numBands, 0.0);
    m_gainReduction.resize(m_numBands, 0.0);
}

void MultibandCompressor3::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_bandParams.size())
        m_bandParams[band] = params;
}

void MultibandCompressor3::setMidSideEnabled(bool enabled) { m_midSide = enabled; }

/* ---- LR4 coefficient computation ---- */

void MultibandCompressor3::computeLR4Coeffs(double freq, double b0[5], double a0[5],
                                              double b1[5], double a1[5]) const
{
    // Linkwitz-Riley 4th order = 2 cascaded 2nd-order Butterworth
    double wc = 2.0 * M_PI * freq / m_sampleRate;
    double cosWc = qCos(wc);
    double sinWc = qSin(wc);
    // Butterworth Q = 0.7071 for 2nd order
    double Q = 0.7071067811865476;
    double alpha = sinWc / (2.0 * Q);

    // Low-pass coefficients (each 2nd-order section)
    double a0n = 1.0 + alpha;
    b0[0] = (1.0 - cosWc) / 2.0 / a0n;
    b0[1] = (1.0 - cosWc) / a0n;
    b0[2] = b0[0];
    a0[0] = 1.0;
    a0[1] = -2.0 * cosWc / a0n;
    a0[2] = (1.0 - alpha) / a0n;

    // High-pass coefficients (each 2nd-order section)
    b1[0] = (1.0 + cosWc) / 2.0 / a0n;
    b1[1] = -(1.0 + cosWc) / a0n;
    b1[2] = b1[0];
    a1[0] = 1.0;
    a1[1] = a0[1];
    a1[2] = a0[2];
}

/* ---- Biquad processing ---- */

double MultibandCompressor3::biquadProcess(double in, BiquadState& s,
                                             const double b[3], const double a[3])
{
    double out = b[0] * in + b[1] * s.x1 + b[2] * s.x2
                 - a[1] * s.y1 - a[2] * s.y2;
    s.x2 = s.x1; s.x1 = in;
    s.y2 = s.y1; s.y1 = out;
    return out;
}

/* ---- Crossover filter for one band ---- */

void MultibandCompressor3::crossoverFilter(const QVector<double>& in,
                                             QVector<double>& lpOut,
                                             QVector<double>& hpOut, int bandIdx)
{
    int n = in.size();
    lpOut.resize(n);
    hpOut.resize(n);

    if (bandIdx >= m_crossoverFreqs.size()) return;

    double lpB[3], lpA[3], hpB[3], hpA[3];
    double b0[5], a0[5], b1[5], a1[5];
    computeLR4Coeffs(m_crossoverFreqs[bandIdx], b0, a0, b1, a1);

    // LP coefficients from b0/a0 (first section), same for second cascade
    lpB[0] = b0[0]; lpB[1] = b0[1]; lpB[2] = b0[2];
    lpA[0] = a0[0]; lpA[1] = a0[1]; lpA[2] = a0[2];
    hpB[0] = b1[0]; hpB[1] = b1[1]; hpB[2] = b1[2];
    hpA[0] = a1[0]; hpA[1] = a1[1]; hpA[2] = a1[2];

    auto& state = m_filterState[bandIdx];
    for (int i = 0; i < n; ++i) {
        // LR4 LP = cascade of 2 LP biquads
        double lp = biquadProcess(in[i], state.lp[0], lpB, lpA);
        lp = biquadProcess(lp, state.lp[1], lpB, lpA);
        lpOut[i] = lp;

        // LR4 HP = cascade of 2 HP biquads
        double hp = biquadProcess(in[i], state.hp[0], hpB, hpA);
        hp = biquadProcess(hp, state.hp[1], hpB, hpA);
        hpOut[i] = hp;
    }
}

/* ---- Gain computation ---- */

double MultibandCompressor3::computeGain(double levelDb, const BandParams& p) const
{
    if (!p.enabled) return 1.0;

    double diff = levelDb - p.threshold;
    if (diff < -p.knee / 2.0) return 1.0; // Below knee

    double targetGain;
    if (qAbs(diff) <= p.knee / 2.0) {
        // Soft knee interpolation
        double x = diff + p.knee / 2.0;
        targetGain = qPow(10.0, (x * x / (2.0 * p.knee)) * (1.0 / p.ratio - 1.0) / 20.0);
    } else {
        double reduction = diff * (1.0 - 1.0 / p.ratio);
        targetGain = qPow(10.0, reduction / 20.0);
    }

    // Apply makeup gain
    double makeup = qPow(10.0, p.makeupGain / 20.0);
    return targetGain * makeup;
}

/* ---- Mid/Side encoding/decoding ---- */

void MultibandCompressor3::encodeMS(const QVector<double>& L, const QVector<double>& R,
                                      QVector<double>& mid, QVector<double>& side)
{
    int n = L.size();
    mid.resize(n); side.resize(n);
    for (int i = 0; i < n; ++i) {
        mid[i] = (L[i] + R[i]) * 0.5;
        side[i] = (L[i] - R[i]) * 0.5;
    }
}

void MultibandCompressor3::decodeMS(const QVector<double>& mid, const QVector<double>& side,
                                      QVector<double>& L, QVector<double>& R)
{
    int n = mid.size();
    L.resize(n); R.resize(n);
    for (int i = 0; i < n; ++i) {
        L[i] = mid[i] + side[i];
        R[i] = mid[i] - side[i];
    }
}

/* ---- Main process ---- */

QVector<QVector<double>> MultibandCompressor3::process(
    const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    int channels = input.size();
    int blockSize = channels > 0 ? input[0].size() : 0;
    if (channels == 0 || blockSize == 0) return input;

    QVector<double> mono(blockSize);
    if (channels == 1) {
        mono = input[0];
    } else {
        for (int i = 0; i < blockSize; ++i)
            mono[i] = (input[0][i] + input[1][i]) * 0.5;
    }

    // Split into frequency bands via cascaded crossovers
    QVector<QVector<double>> bands(m_numBands);
    QVector<double> current = mono;
    for (int b = 0; b < m_crossoverFreqs.size() && b < m_filterState.size(); ++b) {
        QVector<double> lp, hp;
        crossoverFilter(current, lp, hp, b);
        bands[b] = lp;
        current = hp;
    }
    bands[m_numBands - 1] = current;

    // Apply dynamics to each band
    for (int b = 0; b < m_numBands; ++b) {
        const auto& params = m_bandParams[b];
        if (!params.enabled) continue;

        // Envelope detection
        double attackCoeff = qExp(-1.0 / (params.attack * 0.001 * m_sampleRate));
        double releaseCoeff = qExp(-1.0 / (params.release * 0.001 * m_sampleRate));

        double maxGr = 0.0;
        double rmsSum = 0.0;
        for (int i = 0; i < blockSize; ++i) {
            double absVal = qAbs(bands[b][i]);
            if (absVal > m_envLevel[b])
                m_envLevel[b] = attackCoeff * m_envLevel[b] + (1.0 - attackCoeff) * absVal;
            else
                m_envLevel[b] = releaseCoeff * m_envLevel[b] + (1.0 - releaseCoeff) * absVal;

            double levelDb = 20.0 * qLog10(qMax(m_envLevel[b], 1e-10));
            double gain = computeGain(levelDb, params);
            bands[b][i] *= gain;

            double gr = 20.0 * qLog10(qMax(gain, 1e-10));
            maxGr = qMin(maxGr, gr);
            rmsSum += bands[b][i] * bands[b][i];
        }
        m_gainReduction[b] = maxGr;
    }

    // Sum bands back together
    QVector<double> output(blockSize, 0.0);
    for (int b = 0; b < m_numBands; ++b)
        for (int i = 0; i < blockSize; ++i)
            output[i] += bands[b][i];

    QVector<QVector<double>> result;
    if (channels == 1) {
        result = {output};
    } else {
        // For stereo: apply same gain to both channels proportionally
        QVector<double> L(blockSize), R(blockSize);
        for (int i = 0; i < blockSize; ++i) {
            double scale = (qAbs(mono[i]) > 1e-10) ? output[i] / mono[i] : 1.0;
            L[i] = input[0][i] * scale;
            R[i] = input[1][i] * scale;
        }
        result = {L, R};
    }

    m_stats.totalProcessed += blockSize;
    m_stats.numBands = m_numBands;
    m_stats.blockSize = blockSize;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessed > 0)
        ? m_timeSum / m_stats.totalProcessed * blockSize : 0.0;

    double peakRed = 0.0;
    for (double gr : m_gainReduction) peakRed = qMin(peakRed, gr);
    emit processingCompleted(blockSize, peakRed);
    return result;
}

/* ---- Accessors ---- */

QVector<double> MultibandCompressor3::bandLevels() const
{
    QVector<double> levels(m_numBands);
    for (int b = 0; b < m_numBands; ++b)
        levels[b] = 20.0 * qLog10(qMax(m_envLevel[b], 1e-10));
    return levels;
}

QVector<double> MultibandCompressor3::bandGainReduction() const
{
    return m_gainReduction;
}

void MultibandCompressor3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
