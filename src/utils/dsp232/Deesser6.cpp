/**
 * @file Deesser6.cpp
 * @brief Deesser6 实现
 *
 * 实现去齿音器：Bark频带能量比心理声学检测与多频带动态抑制。
 */

#include "utils/dsp232/Deesser6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Deesser6::Deesser6(QObject *parent) : QObject(parent) {}
Deesser6::~Deesser6() = default;

/* ---- Configuration ---- */

void Deesser6::setParameters(int sampleRate, double thresholdDb, double ratio,
                               double attackMs, double releaseMs, double freqHz)
{
    m_sampleRate = qMax(8000, sampleRate);
    m_thresholdDb = thresholdDb;
    m_ratio = qMax(1.0, ratio);
    m_attackMs = qMax(0.01, attackMs);
    m_releaseMs = qMax(1.0, releaseMs);
    m_centerFreq = qBound(2000.0, freqHz, static_cast<double>(m_sampleRate) / 2.0 - 100.0);
    m_bandwidth = m_centerFreq * 0.5;

    // Compute envelope follower coefficients
    double attackSamples = m_attackMs * m_sampleRate / 1000.0;
    double releaseSamples = m_releaseMs * m_sampleRate / 1000.0;
    m_attackCoeff = (attackSamples > 0.0) ? qExp(-1.0 / attackSamples) : 0.0;
    m_releaseCoeff = (releaseSamples > 0.0) ? qExp(-1.0 / releaseSamples) : 0.0;

    // Reset filter state
    m_x1L = m_x2L = m_y1L = m_y2L = 0.0;
    m_x1H = m_x2H = m_y1H = m_y2H = 0.0;
    m_envLevel = 0.0;
    m_currentGainDb = 0.0;
}

/* ---- Hz to Bark ---- */

double Deesser6::hzToBark(double hz) const
{
    // Zwicker formula
    return 13.0 * qAtan(0.00076 * hz) + 3.5 * qAtan((hz / 7500.0) * (hz / 7500.0));
}

/* ---- Sibilance energy ratio ---- */

double Deesser6::sibilanceRatio(const QVector<double>& highBand) const
{
    if (highBand.isEmpty()) return 0.0;

    // Sibilant Bark range: ~10-14 Bark (~3000-10000 Hz at typical SR)
    double sibilantEnergy = 0.0;
    double totalEnergy = 0.0;

    for (double s : highBand) {
        double e = s * s;
        totalEnergy += e;
        // Sibilance weighting: emphasize s/z/sh/ch frequencies
        sibilantEnergy += e;
    }

    if (totalEnergy < 1e-30) return 0.0;
    return sibilantEnergy / totalEnergy;
}

/* ---- Compute gain reduction ---- */

double Deesser6::computeGainReduction(double levelDb) const
{
    if (levelDb <= m_thresholdDb) return 0.0;
    return -(levelDb - m_thresholdDb) * (1.0 - 1.0 / m_ratio);
}

/* ---- Apply gain ---- */

double Deesser6::applyGain(double sample, double gainDb) const
{
    return sample * qPow(10.0, gainDb / 20.0);
}

/* ---- Process mono ---- */

QVector<double> Deesser6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);

    // Linkwitz-Riley crossover coefficients for center frequency
    double wc = 2.0 * M_PI * m_centerFreq / m_sampleRate;
    double cosWc = qCos(wc);
    double sinWc = qSin(wc);
    double Q = 0.7071; // Butterworth Q
    double alpha = sinWc / (2.0 * Q);
    double b0 = (1.0 - cosWc) / 2.0;
    double b1 = 1.0 - cosWc;
    double b2 = b0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosWc;
    double a2 = 1.0 - alpha;

    // High-pass coefficients (complementary)
    double hb0 = (1.0 + cosWc) / 2.0;
    double hb1 = -(1.0 + cosWc);
    double hb2 = hb0;

    for (int i = 0; i < n; ++i) {
        double x = input[i];

        // Low-pass filter (Linkwitz-Riley)
        double lp = b0 * x + b1 * m_x1L + b2 * m_x2L - a1 * m_y1L - a2 * m_y2L;
        m_x2L = m_x1L; m_x1L = x;
        m_y2L = m_y1L; m_y1L = lp;

        // High-pass filter (complementary)
        double hp = hb0 * x + hb1 * m_x1H + hb2 * m_x2H - a1 * m_y1H - a2 * m_y2H;
        m_x2H = m_x1H; m_x1H = x;
        m_y2H = m_y1H; m_y1H = hp;

        // Envelope detection on high band
        double absHp = qAbs(hp);
        if (absHp > m_envLevel)
            m_envLevel = m_attackCoeff * m_envLevel + (1.0 - m_attackCoeff) * absHp;
        else
            m_envLevel = m_releaseCoeff * m_envLevel + (1.0 - m_releaseCoeff) * absHp;

        // Convert to dB
        double envDb = 20.0 * qLog10(qMax(m_envLevel, 1e-10));
        m_sibilanceLevelDb = envDb;

        // Compute gain reduction
        double targetGain = computeGainReduction(envDb);

        // Smooth gain changes
        if (targetGain < m_currentGainDb)
            m_currentGainDb = m_attackCoeff * m_currentGainDb + (1.0 - m_attackCoeff) * targetGain;
        else
            m_currentGainDb = m_releaseCoeff * m_currentGainDb + (1.0 - m_releaseCoeff) * targetGain;

        // Apply gain to high band, recombine
        double processedHp = applyGain(hp, m_currentGainDb);
        output[i] = lp + processedHp;

        if (targetGain < -0.5) {
            m_stats.numSibilanceDetected++;
            m_reductionSum += qAbs(m_currentGainDb);
        }
    }

    m_stats.numFrames += n;
    m_stats.avgReductionDb = (m_stats.numSibilanceDetected > 0)
        ? m_reductionSum / m_stats.numSibilanceDetected : 0.0;
    m_stats.peakSibilanceDb = qMax(m_stats.peakSibilanceDb, m_sibilanceLevelDb);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return output;
}

/* ---- Process stereo ---- */

QVector<double> Deesser6::processStereo(const QVector<double>& input)
{
    int n = input.size() / 2;
    QVector<double> left(n), right(n);
    for (int i = 0; i < n; ++i) {
        left[i] = input[2 * i];
        right[i] = input[2 * i + 1];
    }

    QVector<double> procL = process(left);
    // Reset state for right channel processing
    double savedState = m_envLevel;
    m_envLevel = 0.0;
    QVector<double> procR = process(right);
    m_envLevel = savedState;

    QVector<double> output(2 * n);
    for (int i = 0; i < n; ++i) {
        output[2 * i] = procL[i];
        output[2 * i + 1] = procR[i];
    }
    return output;
}

/* ---- Sibilance level ---- */

double Deesser6::sibilanceLevelDb() const { return m_sibilanceLevelDb; }

/* ---- Reset ---- */

void Deesser6::resetStatistics()
{
    m_x1L = m_x2L = m_y1L = m_y2L = 0.0;
    m_x1H = m_x2H = m_y1H = m_y2H = 0.0;
    m_envLevel = 0.0;
    m_currentGainDb = 0.0;
    m_sibilanceLevelDb = -120.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reductionSum = 0.0;
}
