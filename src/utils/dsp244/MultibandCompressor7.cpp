/**
 * @file MultibandCompressor7.cpp
 * @brief MultibandCompressor7 实现
 *
 * 实现多频段压缩器：Linkwitz-Riley交叉与独立频段阈值/比率/补偿增益。
 */

#include "utils/dsp244/MultibandCompressor7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandCompressor7::MultibandCompressor7(QObject *parent) : QObject(parent)
{
    // Default 4-band setup: crossover at 120, 1000, 4000 Hz
    m_crossoverFreqs = {120.0, 1000.0, 4000.0};
    m_numBands = m_crossoverFreqs.size() + 1;
    m_bands.resize(m_numBands);
    initFilters();
}

MultibandCompressor7::~MultibandCompressor7() = default;

/* ---- Configuration ---- */

void MultibandCompressor7::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    m_stats.sampleRate = m_sampleRate;
    initFilters();
}

void MultibandCompressor7::setCrossoverFreqs(const QVector<double>& freqs)
{
    m_crossoverFreqs = freqs;
    m_numBands = freqs.size() + 1;
    m_bands.resize(m_numBands);
    initFilters();
}

void MultibandCompressor7::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_bands.size())
        m_bands[band].params = params;
}

/* ---- Initialize Linkwitz-Riley filters ---- */

void MultibandCompressor7::initFilters()
{
    // For n bands, we need n-1 crossovers
    // Each crossover splits into LP and HP using 2nd-order Butterworth
    // (cascaded for 4th-order Linkwitz-Riley)
    for (int i = 0; i < m_crossoverFreqs.size(); ++i) {
        if (i < m_bands.size())
            computeLRCoeffs(m_bands[i], m_crossoverFreqs[i]);
    }
    // Mark all bands as ready
    for (auto& b : m_bands)
        b.filterReady = true;
}

/* ---- Compute LR 4th-order coefficients ---- */

void MultibandCompressor7::computeLRCoeffs(BandState& band, double freq)
{
    // 2nd-order Butterworth at freq, will be applied twice for LR4
    double omega = 2.0 * M_PI * freq / m_sampleRate;
    double cosW = qCos(omega);
    double sinW = qSin(omega);
    double alpha = sinW / (qSqrt(2.0));  // Butterworth Q = 1/sqrt(2)

    double a0 = 1.0 + alpha;
    // Low-pass coefficients
    band.b0 = (1.0 - cosW) / 2.0 / a0;
    band.b1 = (1.0 - cosW) / a0;
    band.b2 = (1.0 - cosW) / 2.0 / a0;
    band.a1 = -2.0 * cosW / a0;
    band.a2 = (1.0 - alpha) / a0;
    // High-pass coefficients
    band.hpb0 = (1.0 + cosW) / 2.0 / a0;
    band.hpb1 = -(1.0 + cosW) / a0;
    band.hpb2 = (1.0 + cosW) / 2.0 / a0;
    band.hpa1 = -2.0 * cosW / a0;
    band.hpa2 = (1.0 - alpha) / a0;
}

/* ---- Apply low-pass filter (2nd-order, apply twice for LR4) ---- */

double MultibandCompressor7::applyLP(BandState& band, double x)
{
    // Two cascaded 2nd-order LP sections
    double y = band.b0 * x + band.b1 * band.lpX1[0] + band.b2 * band.lpX2[0]
               - band.a1 * band.lpY1[0] - band.a2 * band.lpY2[0];
    band.lpX2[0] = band.lpX1[0]; band.lpX1[0] = x;
    band.lpY2[0] = band.lpY1[0]; band.lpY1[0] = y;

    double y2 = band.b0 * y + band.b1 * band.lpX1[1] + band.b2 * band.lpX2[1]
                - band.a1 * band.lpY1[1] - band.a2 * band.lpY2[1];
    band.lpX2[1] = band.lpX1[1]; band.lpX1[1] = y;
    band.lpY2[1] = band.lpY1[1]; band.lpY1[1] = y2;

    return y2;
}

/* ---- Apply high-pass filter (2nd-order, apply twice for LR4) ---- */

double MultibandCompressor7::applyHP(BandState& band, double x)
{
    double y = band.hpb0 * x + band.hpb1 * band.hpX1[0] + band.hpb2 * band.hpX2[0]
               - band.hpa1 * band.hpY1[0] - band.hpa2 * band.hpY2[0];
    band.hpX2[0] = band.hpX1[0]; band.hpX1[0] = x;
    band.hpY2[0] = band.hpY1[0]; band.hpY1[0] = y;

    double y2 = band.hpb0 * y + band.hpb1 * band.hpX1[1] + band.hpb2 * band.hpX2[1]
                - band.hpa1 * band.hpY1[1] - band.hpa2 * band.hpY2[1];
    band.hpX2[1] = band.hpX1[1]; band.hpX1[1] = y;
    band.hpY2[1] = band.hpY1[1]; band.hpY1[1] = y2;

    return y2;
}

/* ---- Compute gain reduction ---- */

double MultibandCompressor7::computeGain(BandState& band, double input)
{
    const auto& p = band.params;
    // Convert input to dB
    double absInput = qFabs(input);
    double inputDb = absInput > 1e-10 ? 20.0 * qLn(absInput) / M_LN10 : -200.0;

    // Envelope follower with attack/release
    double coeff = inputDb > band.envelope
        ? 1.0 - qExp(-1.0 / (p.attack * m_sampleRate * 0.001))
        : 1.0 - qExp(-1.0 / (p.release * m_sampleRate * 0.001));
    band.envelope += coeff * (inputDb - band.envelope);

    // Soft knee compression
    double halfKnee = p.kneeWidth / 2.0;
    double thresholdMinusKnee = p.threshold - halfKnee;
    double thresholdPlusKnee = p.threshold + halfKnee;
    double gainDb = 0.0;

    if (band.envelope < thresholdMinusKnee) {
        gainDb = 0.0;
    } else if (band.envelope > thresholdPlusKnee) {
        gainDb = (p.threshold + (band.envelope - p.threshold) / p.ratio) - band.envelope;
    } else {
        // Quadratic interpolation in soft knee region
        double x = band.envelope - thresholdMinusKnee;
        double t = x / p.kneeWidth;
        double slope = (1.0 / p.ratio - 1.0);
        gainDb = x * x * slope / (2.0 * p.kneeWidth);
    }

    // Apply makeup gain
    gainDb += p.makeupGain;
    band.gainReduction = gainDb;

    // Convert back to linear gain
    return qPow(10.0, gainDb / 20.0);
}

/* ---- Process single sample ---- */

double MultibandCompressor7::processOne(double sample)
{
    if (m_bands.isEmpty()) return sample;

    double output = 0.0;
    double currentSignal = sample;

    // Cascaded crossover: split signal band by band
    for (int i = 0; i < m_bands.size(); ++i) {
        double bandSignal;
        if (i < m_crossoverFreqs.size()) {
            // Split into LP (this band) and HP (pass to next)
            bandSignal = applyLP(m_bands[i], currentSignal);
            currentSignal = applyHP(m_bands[i], currentSignal);
        } else {
            // Last band gets remaining high-pass signal
            bandSignal = currentSignal;
        }

        // Apply compression to this band
        double gain = computeGain(m_bands[i], bandSignal);
        output += bandSignal * gain;
    }

    return output;
}

/* ---- Process block ---- */

QVector<double> MultibandCompressor7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(input.size());
    for (double s : input)
        output.append(processOne(s));

    m_stats.numBands = m_bands.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit blockProcessed(input.size(), m_bands.size(), timer.elapsed());
    return output;
}

/* ---- Gain reductions ---- */

QVector<double> MultibandCompressor7::gainReductions() const
{
    QVector<double> gr;
    gr.reserve(m_bands.size());
    for (const auto& b : m_bands)
        gr.append(b.gainReduction);
    return gr;
}

/* ---- Reset ---- */

void MultibandCompressor7::resetStatistics()
{
    for (auto& b : m_bands) {
        b = BandState{};
        b.filterReady = false;
    }
    initFilters();
    m_stats = Stats{}; m_timeSum = 0.0;
}
