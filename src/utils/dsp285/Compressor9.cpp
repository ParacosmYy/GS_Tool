/**
 * @file Compressor9.cpp
 * @brief Compressor9 实现
 *
 * 实现多频段动态压缩器：侧链与可变交叉淡入比的透明频段相关动态处理。
 */

#include "utils/dsp285/Compressor9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Compressor9::Compressor9(int numBands, QObject *parent)
    : QObject(parent), m_numBands(qBound(1, numBands, 8))
{
    m_bands.resize(m_numBands);
    m_envState.resize(m_numBands, 0.0);

    // Default band crossover frequencies
    double freqs[] = {120.0, 500.0, 2000.0, 6000.0, 10000.0, 14000.0, 18000.0};
    for (int i = 0; i < m_numBands; ++i) {
        m_bands[i].lowFreq = (i > 0) ? freqs[i - 1] : 0.0;
        m_bands[i].highFreq = (i < m_numBands - 1) ? freqs[i] : 22050.0;
    }
    designCrossovers();
}

Compressor9::~Compressor9() = default;

/* ---- Configuration ---- */

void Compressor9::setSampleRate(double sr) { m_sampleRate = qBound(8000.0, sr, 192000.0); designCrossovers(); }
void Compressor9::setCrossfadeRatio(double ratio) { m_crossfade = qBound(0.0, ratio, 1.0); }

void Compressor9::setBandParams(int band, const BandParams& params)
{
    if (band < 0 || band >= m_numBands) return;
    m_bands[band] = params;
}

void Compressor9::setSidechainSource(const QVector<double>& sc) { m_sidechain = sc; }

/* ---- dB conversions ---- */

double Compressor9::toDb(double linear) { return 20.0 * qLn(qMax(qAbs(linear), 1e-30)) / M_LN2; }
double Compressor9::fromDb(double db) { return qExp(db * M_LN2 / 20.0); }

/* ---- Design crossover filters ---- */

void Compressor9::designCrossovers()
{
    // Simplified Linkwitz-Riley 2nd-order crossover coefficients
    // Stores {b0, b1, b2, a1, a2} for each crossover point
    int numXovers = qMax(0, m_numBands - 1);
    m_crossoverCoeffs.resize(numXovers);

    for (int i = 0; i < numXovers; ++i) {
        double fc = m_bands[i].highFreq;
        if (fc <= 0.0) fc = 1000.0;
        double omega = 2.0 * M_PI * fc / m_sampleRate;
        double cosW = qCos(omega);
        double sinW = qSin(omega);
        double Q = 0.7071;  // Butterworth Q
        double alpha = sinW / (2.0 * Q);

        double a0 = 1.0 + alpha;
        QVector<double> lpf = {
            (1.0 - cosW) / (2.0 * a0),   // b0
            (1.0 - cosW) / a0,            // b1
            (1.0 - cosW) / (2.0 * a0),   // b2
            -2.0 * cosW / a0,             // a1
            (1.0 - alpha) / a0            // a2
        };
        m_crossoverCoeffs[i] = lpf;
    }

    // Reset filter states
    m_filterStates.resize(numXovers);
    for (auto& states : m_filterStates) {
        states.resize(2);  // LP + HP per crossover
        for (auto& s : states) s = FilterState{};
    }
}

/* ---- Apply crossover filter bank ---- */

QVector<QVector<double>> Compressor9::applyCrossoverBank(const QVector<double>& input)
{
    int n = input.size();
    QVector<QVector<double>> bands(m_numBands);

    if (m_numBands == 1) {
        bands[0] = input;
        return bands;
    }

    // Simple band-split: use LP/HP per crossover
    QVector<double> current = input;
    for (int i = 0; i < m_numBands - 1; ++i) {
        bands[i].resize(n);
        QVector<double> hp(n);

        if (i < m_crossoverCoeffs.size()) {
            const auto& coeff = m_crossoverCoeffs[i];
            auto& lpState = m_filterStates[i][0];
            auto& hpState = m_filterStates[i][1];

            for (int s = 0; s < n; ++s) {
                double x = current[s];
                double lpY = coeff[0] * x + coeff[1] * lpState.x1 + coeff[2] * lpState.x2
                             - coeff[3] * lpState.y1 - coeff[4] * lpState.y2;
                lpState.x2 = lpState.x1; lpState.x1 = x;
                lpState.y2 = lpState.y1; lpState.y1 = lpY;

                double hpY = x - lpY;  // Complementary HP
                hpState = hpState;     // Track state

                bands[i][s] = lpY;
                hp[s] = hpY;
            }
        } else {
            bands[i] = current;
            hp.fill(0.0);
        }
        current = hp;
    }
    bands[m_numBands - 1] = current;

    return bands;
}

/* ---- Process envelope for single band ---- */

double Compressor9::processEnvelope(int band, double input, double samplePeriod)
{
    const BandParams& p = m_bands[band];
    double level = qAbs(input);
    double attackCoeff = qExp(-1.0 / (p.attack * 0.001 * m_sampleRate));
    double releaseCoeff = qExp(-1.0 / (p.release * 0.001 * m_sampleRate));

    if (level > m_envState[band])
        m_envState[band] = attackCoeff * m_envState[band] + (1.0 - attackCoeff) * level;
    else
        m_envState[band] = releaseCoeff * m_envState[band] + (1.0 - releaseCoeff) * level;

    return m_envState[band];
}

/* ---- Compute gain reduction ---- */

double Compressor9::computeGainReduction(double inputDb, const BandParams& band) const
{
    double threshold = band.threshold;
    double ratio = band.ratio;
    double knee = band.knee;

    double overDb = inputDb - threshold;
    if (knee > 0.0 && overDb > -knee / 2.0 && overDb < knee / 2.0) {
        // Soft knee: quadratic interpolation
        double x = overDb + knee / 2.0;
        overDb = x * x / (2.0 * knee);
    }

    if (overDb <= 0.0) return 0.0;

    double gr = overDb * (1.0 - 1.0 / ratio);
    return -gr;
}

/* ---- Main process ---- */

Compressor9::ProcessResult Compressor9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    int n = input.size();
    if (n == 0) return result;

    // Split into bands
    auto bandSignals = applyCrossoverBank(input);

    // Apply compression per band
    result.output.resize(n, 0.0);
    result.gainReduction.resize(n, 0.0);
    double peakRed = 0.0;
    double outPeak = 0.0;
    double samplePeriod = 1.0 / m_sampleRate;

    for (int band = 0; band < m_numBands; ++band) {
        const BandParams& p = m_bands[band];
        for (int i = 0; i < n; ++i) {
            double src = (i < m_sidechain.size()) ? m_sidechain[i] : bandSignals[band][i];

            // Envelope follower
            double env = processEnvelope(band, src, samplePeriod);
            double envDb = toDb(env);

            // Gain reduction
            double grDb = computeGainReduction(envDb, p);
            double grLinear = fromDb(grDb + p.makeupGain);

            // Apply with mix
            double compressed = bandSignals[band][i] * grLinear;
            double out = p.mix * compressed + (1.0 - p.mix) * bandSignals[band][i];
            result.output[i] += out;
            result.gainReduction[i] += grDb;

            if (qAbs(grDb) > peakRed) peakRed = qAbs(grDb);
            if (qAbs(out) > outPeak) outPeak = qAbs(out);
        }
    }

    result.peakReduction = peakRed;
    result.outputPeak = outPeak;

    double elapsed = timer.elapsed();
    m_stats.numBands = m_numBands;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processDone(n, m_numBands, peakRed, elapsed);

    return result;
}

/* ---- Reset ---- */

void Compressor9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envState.fill(0.0);
    for (auto& states : m_filterStates)
        for (auto& s : states) s = FilterState{};
}
