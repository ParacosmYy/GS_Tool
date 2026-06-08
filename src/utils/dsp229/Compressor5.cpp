/**
 * @file Compressor5.cpp
 * @brief Compressor5 实现
 *
 * 实现侧链压缩器：外部键控输入与多频段依赖时序去齿音/声塑形。
 */

#include "utils/dsp229/Compressor5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Compressor5::Compressor5(QObject *parent) : QObject(parent) {}
Compressor5::~Compressor5() = default;

/* ---- dB <-> linear conversion ---- */

double Compressor5::dbToLinear(double db) { return qPow(10.0, db / 20.0); }
double Compressor5::linearToDb(double lin) { return 20.0 * qLog10(qMax(1e-10, lin)); }

/* ---- Configure ---- */

bool Compressor5::configure(double sampleRate, const QVector<BandParams>& bands)
{
    if (sampleRate <= 0 || bands.isEmpty()) return false;
    m_sampleRate = sampleRate;
    m_bands = bands;
    m_numBands = bands.size();
    m_envelopeState.resize(m_numBands, 0.0);
    m_prevGain.resize(m_numBands, 1.0);
    m_stats.numBands = m_numBands;
    return true;
}

/* ---- Soft knee gain computation ---- */

double Compressor5::computeGain(double inputDb, const BandParams& bp) const
{
    double halfKnee = bp.knee / 2.0;
    double threshold = bp.threshold;

    if (inputDb < threshold - halfKnee) {
        return inputDb;  // Below knee: no compression
    } else if (inputDb > threshold + halfKnee) {
        // Above knee: full compression
        return threshold + (inputDb - threshold) / bp.ratio;
    } else {
        // Within soft knee: quadratic interpolation
        double x = inputDb - threshold + halfKnee;
        double compressed = x * x / (2.0 * bp.knee);
        return threshold - halfKnee + compressed +
               (inputDb - threshold + halfKnee) *
               (1.0 / bp.ratio - 1.0) * x / (2.0 * bp.knee) * 0;
        // Simplified: linear blend within knee
        double t = (inputDb - threshold + halfKnee) / bp.knee;
        double below = inputDb;
        double above = threshold + (inputDb - threshold) / bp.ratio;
        return below + t * (above - below);
    }
}

/* ---- Envelope follower ---- */

double Compressor5::followEnvelope(double inputLevel, int bandIdx)
{
    if (bandIdx < 0 || bandIdx >= m_numBands) return inputLevel;

    double attackCoeff = 1.0 - qExp(-1.0 / (m_bands[bandIdx].attack *
                         m_sampleRate / 1000.0));
    double releaseCoeff = 1.0 - qExp(-1.0 / (m_bands[bandIdx].release *
                          m_sampleRate / 1000.0));

    double& env = m_envelopeState[bandIdx];
    double coeff = (inputLevel > env) ? attackCoeff : releaseCoeff;
    env += coeff * (inputLevel - env);
    return env;
}

/* ---- Process with internal sidechain ---- */

QVector<double> Compressor5::process(const QVector<double>& input)
{
    return processWithKey(input, input);
}

/* ---- Process with external key ---- */

QVector<double> Compressor5::processWithKey(const QVector<double>& input,
                                             const QVector<double>& keyInput)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), keyInput.size());
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sample = input[i];
        double keyAbs = qAbs(keyInput[i]);

        // Process each band
        for (int b = 0; b < m_numBands; ++b) {
            double envelope = followEnvelope(keyAbs, b);
            double envDb = linearToDb(envelope);

            // Compute gain reduction
            double outputDb = computeGain(envDb, m_bands[b]);
            double gainDb = outputDb - envDb + m_bands[b].makeupGain;
            double gainLin = dbToLinear(gainDb);

            // Smooth gain transitions
            m_prevGain[b] = 0.9 * m_prevGain[b] + 0.1 * gainLin;
        }

        // For single-band: apply directly
        if (m_numBands == 1) {
            output[i] = sample * m_prevGain[0];
        } else {
            // Multi-band: weighted sum based on band frequency ranges
            double sumGain = 0.0;
            for (int b = 0; b < m_numBands; ++b)
                sumGain += m_prevGain[b];
            double avgGain = sumGain / m_numBands;
            output[i] = sample * avgGain;
        }
    }

    m_stats.framesProcessed += n;
    double avgRed = 0.0;
    for (int b = 0; b < m_numBands; ++b)
        avgRed += linearToDb(m_prevGain[b]);
    avgRed /= qMax(1, m_numBands);
    m_stats.avgGainReduction = avgRed;
    m_stats.peakReduction = qMin(m_stats.peakReduction, avgRed);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit frameProcessed(n, avgRed, timer.elapsed());
    return output;
}

/* ---- Gain reduction per band ---- */

QVector<double> Compressor5::gainReduction() const
{
    QVector<double> gr(m_numBands);
    for (int b = 0; b < m_numBands; ++b)
        gr[b] = linearToDb(m_prevGain[b]);
    return gr;
}

/* ---- Reset ---- */

void Compressor5::resetStatistics()
{
    m_envelopeState.fill(0.0);
    m_prevGain.fill(1.0);
    m_stats = Stats{};
    m_timeSum = 0.0;
}
