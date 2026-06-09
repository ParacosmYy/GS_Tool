/**
 * @file Expander10.cpp
 * @brief Expander10 实现
 *
 * 实现多频段扩展器：对数频段分割与独立时间常数逐频段处理。
 */

#include "utils/dsp249/Expander10.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Expander10::Expander10(QObject *parent) : QObject(parent) {}
Expander10::~Expander10() = default;

/* ---- Configuration ---- */

void Expander10::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }

void Expander10::setBands(const QVector<BandConfig>& bands)
{
    m_bands.clear();
    for (const auto& cfg : bands) {
        BandState state;
        state.config = cfg;
        state.envelope = 0.0;
        state.gainLinear = 1.0;
        state.filterCoeff = {};
        state.level = {};
        m_bands.append(state);
    }
    m_stats.numBands = m_bands.size();
}

/* ---- Compute logarithmic band edges ---- */

QVector<QPair<double, double>> Expander10::logBandEdges(
    int numBands, double minFreq, double maxFreq) const
{
    QVector<QPair<double, double>> edges;
    double logMin = qLn(qMax(20.0, minFreq));
    double logMax = qLn(qMax(21.0, maxFreq));
    double step = (logMax - logMin) / numBands;

    for (int i = 0; i < numBands; ++i) {
        double lo = qExp(logMin + i * step);
        double hi = (i == numBands - 1) ? maxFreq : qExp(logMin + (i + 1) * step);
        edges.append({lo, hi});
    }
    return edges;
}

/* ---- Simple bandpass coefficient ---- */

double Expander10::bandpassCoeff(double lowFreq, double highFreq,
                                  double sampleRate) const
{
    double center = qSqrt(qMax(1.0, lowFreq * highFreq));
    double bw = qLn(qMax(1.001, highFreq / qMax(1.0, lowFreq)));
    // Simple 2nd-order coefficient approximation
    double omega = 2.0 * M_PI * center / sampleRate;
    double alpha = qSin(omega) * qSinh(bw / 2.0);
    return (omega > 0 && alpha > 0) ? alpha / (1.0 + alpha) : 0.5;
}

/* ---- Compute expander gain ---- */

double Expander10::computeGain(double inputDb, const BandConfig& cfg) const
{
    // Below threshold: apply expansion ratio
    if (inputDb < cfg.threshold) {
        double diff = cfg.threshold - inputDb;
        double expandedDb = cfg.threshold - diff * cfg.ratio;
        return qPow(10.0, expandedDb / 20.0);
    }
    // Above threshold: unity gain
    return 1.0;
}

/* ---- Update envelope follower ---- */

double Expander10::updateEnvelope(double input, double envelope,
                                   double attackCoeff, double releaseCoeff) const
{
    double absInput = qAbs(input);
    double coeff = (absInput > envelope) ? attackCoeff : releaseCoeff;
    return envelope + coeff * (absInput - envelope);
}

/* ---- Process block ---- */

QVector<double> Expander10::processBlock(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    int n = samples.size();
    QVector<double> output(n, 0.0);

    if (m_bands.isEmpty()) {
        output = samples;
    } else {
        for (int b = 0; b < m_bands.size(); ++b) {
            auto& band = m_bands[b];
            double attackCoeff = 1.0 - qExp(-1.0 / (band.config.attack * 0.001 * m_sampleRate));
            double releaseCoeff = 1.0 - qExp(-1.0 / (band.config.release * 0.001 * m_sampleRate));
            double bpCoeff = bandpassCoeff(band.config.lowFreq, band.config.highFreq, m_sampleRate);

            double bandEnergy = 0.0;
            for (int i = 0; i < n; ++i) {
                // Simple bandpass via single-pole filter
                double filtered = samples[i] * bpCoeff;
                band.envelope = updateEnvelope(filtered, band.envelope,
                                                attackCoeff, releaseCoeff);

                // Convert envelope to dB
                double envDb = (band.envelope > 1e-10)
                    ? 20.0 * qLn(band.envelope) / qLn(10.0) : -120.0;

                // Compute gain
                band.gainLinear = computeGain(envDb, band.config);
                output[i] += filtered * band.gainLinear;
                bandEnergy += band.envelope;
            }

            band.level.inputDb = (bandEnergy / n > 1e-10)
                ? 20.0 * qLn(bandEnergy / n) / qLn(10.0) : -120.0;
            band.level.gainDb = 20.0 * qLn(qMax(1e-10, band.gainLinear)) / qLn(10.0);
        }
    }

    m_stats.blockSize = n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit blockProcessed(n, m_bands.size(), timer.elapsed());
    return output;
}

/* ---- Get band levels ---- */

QVector<Expander10::BandLevel> Expander10::bandLevels() const
{
    QVector<BandLevel> levels;
    for (const auto& band : m_bands)
        levels.append(band.level);
    return levels;
}

/* ---- Reset ---- */

void Expander10::resetStatistics()
{
    for (auto& band : m_bands) {
        band.envelope = 0.0;
        band.gainLinear = 1.0;
        band.level = {};
    }
    m_stats = Stats{};
    m_timeSum = 0.0;
}
