/**
 * @file MultibandCompressor5.cpp
 * @brief MultibandCompressor5 实现
 *
 * 实现多频段压缩：LR4交叉滤波器、逐频段压缩、自动补偿增益。
 */

#include "utils/dsp216/MultibandCompressor5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandCompressor5::MultibandCompressor5(QObject *parent) : QObject(parent)
{
    // Default 4-band with crossover at 120, 500, 4000 Hz
    setParameters(44100.0, {120.0, 500.0, 4000.0});
}

MultibandCompressor5::~MultibandCompressor5() = default;

/* ---- Configuration ---- */

void MultibandCompressor5::setParameters(double sampleRate,
                                          const QVector<double>& crossoverFreqs)
{
    m_sampleRate = qMax(8000.0, sampleRate);
    m_numBands = crossoverFreqs.size() + 1;
    m_crossoverState.resize(crossoverFreqs.size());
    m_bandParams.resize(m_numBands);
    m_envelope.resize(m_numBands, -60.0);
    m_gainRed.resize(m_numBands, 0.0);

    // Default band params
    for (int i = 0; i < m_numBands; ++i) {
        m_bandParams[i].threshold = -20.0 - i * 5.0;
        m_bandParams[i].ratio = 4.0;
        m_bandParams[i].attack = 5.0;
        m_bandParams[i].release = 50.0;
        m_bandParams[i].autoMatch = true;
    }

    m_stats.numBands = m_numBands;
    m_stats.sampleRate = m_sampleRate;
}

void MultibandCompressor5::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_numBands)
        m_bandParams[band] = params;
}

/* ---- LR4 crossover coefficients ---- */

void MultibandCompressor5::computeLRCoeffs(double freq, double sampleRate,
                                             QVector<double>& b,
                                             QVector<double>& a) const
{
    // 2nd-order Butterworth lowpass coefficients
    double omega = 2.0 * M_PI * freq / sampleRate;
    double sinO = qSin(omega);
    double cosO = qCos(omega);
    double alpha = sinO / (2.0 * qSqrt(0.5));  // Butterworth Q=0.7071

    double a0 = 1.0 + alpha;
    b = {(1.0 - cosO) / 2.0, (1.0 - cosO), (1.0 - cosO) / 2.0};
    a = {1.0, -2.0 * cosO / a0, (1.0 - alpha) / a0};
    for (auto& v : b) v /= a0;
}

/* ---- Apply single biquad ---- */

double MultibandCompressor5::applyBiquad(double x, const QVector<double>& b,
                                          const QVector<double>& a,
                                          BiquadState& st) const
{
    double y = b[0] * x + b[1] * st.x1 + b[2] * st.x2
               - a[1] * st.y1 - a[2] * st.y2;
    st.x2 = st.x1;
    st.x1 = x;
    st.y2 = st.y1;
    st.y1 = y;
    return y;
}

/* ---- Compute gain reduction ---- */

double MultibandCompressor5::computeGainReduction(int band,
                                                    double inputLevel) const
{
    const auto& bp = m_bandParams[band];
    double thDb = bp.threshold;
    double ratio = bp.ratio;

    if (inputLevel <= thDb) return 0.0;

    // Compressor: output = threshold + (input - threshold) / ratio
    double overDb = inputLevel - thDb;
    double reduction = overDb * (1.0 - 1.0 / ratio);
    return -reduction;
}

/* ---- Auto-match makeup gain ---- */

double MultibandCompressor5::autoMatchGain(int band, double inputLevel,
                                            double outputLevel) const
{
    Q_UNUSED(band)
    // Match output level to input level
    return inputLevel - outputLevel;
}

/* ---- Process single sample ---- */

double MultibandCompressor5::processSample(double sample)
{
    // Split into bands using LR4 crossovers
    QVector<double> bandSignals(m_numBands);

    // First band: chain of LP filters
    double lp = sample;
    double hp = sample;
    for (int i = 0; i < m_crossoverState.size(); ++i) {
        QVector<double> bLP, aLP, bHP, aHP;
        double freq = 120.0 * qPow(500.0 / 120.0,
                                     static_cast<double>(i) / qMax(1, m_crossoverState.size() - 1));
        if (i == 0) freq = 120.0;
        else if (i == 1) freq = 500.0;
        else if (i == 2) freq = 4000.0;
        else freq = 4000.0 * (i - 1);

        // Compute coefficients
        double omega = 2.0 * M_PI * freq / m_sampleRate;
        double cosO = qCos(omega);
        double sinO = qSin(omega);
        double alpha = sinO / qSqrt(2.0);
        double a0 = 1.0 + alpha;

        QVector<double> bL = {(1.0 - cosO) / 2.0 / a0,
                               (1.0 - cosO) / a0,
                               (1.0 - cosO) / 2.0 / a0};
        QVector<double> aL = {1.0, -2.0 * cosO / a0, (1.0 - alpha) / a0};
        QVector<double> bH = {(1.0 + cosO) / 2.0 / a0,
                               -(1.0 + cosO) / a0,
                               (1.0 + cosO) / 2.0 / a0};
        QVector<double> aH = aL;

        // Two cascaded biquads for LR4
        double lpOut = applyBiquad(applyBiquad(hp, bL, aL, m_crossoverState[i].lp1),
                                    bL, aL, m_crossoverState[i].lp2);
        double hpOut = applyBiquad(applyBiquad(hp, bH, aH, m_crossoverState[i].hp1),
                                    bH, aH, m_crossoverState[i].hp2);

        bandSignals[i] = lpOut;
        hp = hpOut;
    }
    bandSignals[m_numBands - 1] = hp;

    // Apply per-band compression
    double output = 0.0;
    for (int b = 0; b < m_numBands; ++b) {
        double levelDb = 20.0 * qLn(qMax(qAbs(bandSignals[b]), 1e-10)) / M_LN2 / 6.02;

        // Envelope follower
        double attackCoeff = qExp(-1.0 / (m_bandParams[b].attack * m_sampleRate / 1000.0));
        double releaseCoeff = qExp(-1.0 / (m_bandParams[b].release * m_sampleRate / 1000.0));
        double coeff = (levelDb > m_envelope[b]) ? attackCoeff : releaseCoeff;
        m_envelope[b] = levelDb + coeff * (m_envelope[b] - levelDb);

        double gr = computeGainReduction(b, m_envelope[b]);
        m_gainRed[b] = gr;

        double gainLin = qPow(10.0, (gr + m_bandParams[b].makeupGain) / 20.0);
        output += bandSignals[b] * gainLin;
    }

    m_stats.totalSamples++;
    return output;
}

/* ---- Process block ---- */

QVector<double> MultibandCompressor5::processBlock(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(samples.size());
    for (int i = 0; i < samples.size(); ++i)
        output.append(processSample(samples[i]));

    m_stats.blockSize = samples.size();
    m_stats.totalSamples += samples.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples / 256);
    emit blockProcessed(m_numBands, timer.elapsed());
    return output;
}

/* ---- Band levels ---- */

QVector<double> MultibandCompressor5::bandLevels() const { return m_envelope; }
QVector<double> MultibandCompressor5::gainReduction() const { return m_gainRed; }

/* ---- Reset ---- */

void MultibandCompressor5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope.fill(-60.0);
    m_gainRed.fill(0.0);
    for (auto& cs : m_crossoverState) {
        cs = LR4State{};
    }
}
