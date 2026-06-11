/**
 * @file MultibandCompressor10.cpp
 * @brief MultibandCompressor10 实现
 *
 * 实现多频段压缩器：动态频段分割与每频段自动补偿增益的母带级动态控制。
 */

#include "utils/dsp286/MultibandCompressor10.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

MultibandCompressor10::MultibandCompressor10(int numBands, QObject *parent)
    : QObject(parent), m_numBands(qBound(2, numBands, 8))
{
    setNumBands(m_numBands);
}

MultibandCompressor10::~MultibandCompressor10() = default;

/* ---- Configuration ---- */

void MultibandCompressor10::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
    initFilters();
}

void MultibandCompressor10::setNumBands(int n)
{
    m_numBands = qBound(2, n, 8);
    m_params.resize(m_numBands);
    m_bandStates.resize(m_numBands);

    // Default crossover frequencies (logarithmically spaced)
    m_crossoverFreqs.resize(m_numBands - 1);
    double loFreq = 80.0, hiFreq = 12000.0;
    for (int i = 0; i < m_numBands - 1; ++i) {
        double ratio = static_cast<double>(i + 1) / m_numBands;
        m_crossoverFreqs[i] = loFreq * qPow(hiFreq / loFreq, ratio);
    }
    initFilters();
}

void MultibandCompressor10::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_numBands)
        m_params[band] = params;
}

void MultibandCompressor10::setCrossoverFreqs(const QVector<double>& freqs)
{
    if (freqs.size() == m_numBands - 1)
        m_crossoverFreqs = freqs;
    initFilters();
}

/* ---- Design Linkwitz-Riley 2nd-order crossover ---- */

void MultibandCompressor10::designCrossover(double freq, FilterState& lp, FilterState& hp)
{
    double wc = 2.0 * M_PI * freq / m_sampleRate;
    double cosWc = qCos(wc);
    double sinWc = qSin(wc);
    double alpha = sinWc * qSqrt(2.0) / 2.0;

    // Butterworth 2nd-order low-pass
    double norm = 1.0 + alpha;
    lp.b0 = (1.0 - cosWc) / 2.0 / norm;
    lp.b1 = (1.0 - cosWc) / norm;
    lp.b2 = lp.b0;
    lp.a1 = -2.0 * cosWc / norm;
    lp.a2 = (1.0 - alpha) / norm;

    // Butterworth 2nd-order high-pass
    hp.b0 = (1.0 + cosWc) / 2.0 / norm;
    hp.b1 = -(1.0 + cosWc) / norm;
    hp.b2 = hp.b0;
    hp.a1 = -2.0 * cosWc / norm;
    hp.a2 = (1.0 - alpha) / norm;
}

/* ---- Apply biquad filter ---- */

double MultibandCompressor10::applyFilter(FilterState& fs, double x)
{
    double y = fs.b0 * x + fs.b1 * fs.x1 + fs.b2 * fs.x2
               - fs.a1 * fs.y1 - fs.a2 * fs.y2;
    fs.x2 = fs.x1; fs.x1 = x;
    fs.y2 = fs.y1; fs.y1 = y;
    return y;
}

/* ---- Initialize filter states ---- */

void MultibandCompressor10::initFilters()
{
    for (int b = 0; b < m_numBands; ++b) {
        m_bandStates[b].lpFilter.resize(2);
        m_bandStates[b].hpFilter.resize(2);
        m_bandStates[b].envDb = -120.0;
        m_bandStates[b].gainLin = 1.0;
        m_bandStates[b].sampleRate = m_sampleRate;
    }
    // Design crossover filters for each band boundary
    for (int i = 0; i < m_crossoverFreqs.size(); ++i) {
        FilterState lp, hp;
        designCrossover(m_crossoverFreqs[i], lp, hp);
        // Assign LP to band i, HP to band i+1
        m_bandStates[i].lpFilter[0] = lp;
        m_bandStates[i + 1].hpFilter[0] = hp;
    }
}

/* ---- Compute gain reduction ---- */

double MultibandCompressor10::computeGainReduction(int band, double inputDb)
{
    const BandParams& p = m_params[band];
    if (inputDb <= p.threshold) return 0.0;

    // Compression: gain reduction in dB
    double overDb = inputDb - p.threshold;
    double reductionDb = overDb * (1.0 - 1.0 / p.ratio);

    // Apply attack/release envelope
    BandState& bs = m_bandStates[band];
    double coeff;
    if (reductionDb > 0.0) {
        // Attack: moving toward more reduction
        double attackCoeff = qExp(-1.0 / (p.attack * m_sampleRate / 1000.0));
        coeff = attackCoeff;
    } else {
        // Release: moving toward less reduction
        double releaseCoeff = qExp(-1.0 / (p.release * m_sampleRate / 1000.0));
        coeff = releaseCoeff;
    }

    double targetDb = -reductionDb;
    bs.envDb = coeff * bs.envDb + (1.0 - coeff) * targetDb;

    return bs.envDb;
}

/* ---- Process single sample ---- */

MultibandCompressor10::FrameOutput MultibandCompressor10::processSample(double sample)
{
    FrameOutput out;
    out.bandLevels.resize(m_numBands);

    QVector<double> bandSignals(m_numBands, 0.0);

    // Apply crossover filters (cascaded)
    double sig = sample;
    for (int b = 0; b < m_numBands; ++b) {
        double filtered = sig;
        if (b < m_numBands - 1 && !m_bandStates[b].lpFilter.isEmpty())
            filtered = applyFilter(m_bandStates[b].lpFilter[0], filtered);
        if (b > 0 && !m_bandStates[b].hpFilter.isEmpty())
            filtered = applyFilter(m_bandStates[b].hpFilter[0], filtered);
        bandSignals[b] = filtered;
    }

    // Compress each band
    double output = 0.0;
    for (int b = 0; b < m_numBands; ++b) {
        double absVal = qAbs(bandSignals[b]);
        double inputDb = (absVal > 1e-10) ? 20.0 * qLn(absVal) / M_LN10 : -120.0;

        double gainReduction = computeGainReduction(b, inputDb);
        double makeup = m_params[b].autoMakeup ? m_params[b].makeupGain : m_params[b].makeupGain;
        double totalGainDb = gainReduction + makeup;
        double gainLin = qPow(10.0, totalGainDb / 20.0);

        double bandOut = bandSignals[b] * gainLin;
        output += bandOut;

        out.bandLevels[b].inputDb = inputDb;
        out.bandLevels[b].gainReduction = gainReduction;
        out.bandLevels[b].makeupGain = makeup;
        double outAbs = qAbs(bandOut);
        out.bandLevels[b].outputDb = (outAbs > 1e-10) ? 20.0 * qLn(outAbs) / M_LN10 : -120.0;
    }
    out.sample = output;
    m_stats.totalSamples++;
    return out;
}

/* ---- Process block ---- */

QVector<MultibandCompressor10::FrameOutput> MultibandCompressor10::processBlock(const QVector<double>& block)
{
    QElapsedTimer timer;
    timer.start();

    QVector<FrameOutput> results;
    results.reserve(block.size());
    for (double s : block)
        results.append(processSample(s));

    double elapsed = timer.elapsed();
    m_stats.numBands = m_numBands;
    m_stats.totalSamples += block.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples / 1000);
    emit blockDone(block.size(), elapsed);

    return results;
}

/* ---- Compute automatic makeup gain ---- */

void MultibandCompressor10::computeAutoMakeup()
{
    for (int b = 0; b < m_numBands; ++b) {
        // Estimate average gain reduction and compensate
        double avgReduction = m_params[b].threshold * (1.0 - 1.0 / m_params[b].ratio) * 0.5;
        m_params[b].makeupGain = qMax(0.0, -avgReduction * 0.5);
    }
}

/* ---- Reset ---- */

void MultibandCompressor10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    initFilters();
}
