/**
 * @file Gate3.cpp
 * @brief Gate3 实现
 *
 * 实现噪声门：频谱检测、自适应阈值、双频段独立门控。
 */

#include "utils/dsp178/Gate3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Gate3::Gate3(QObject *parent)
    : QObject(parent)
{
}

Gate3::~Gate3() = default;

/* ---- Configuration ---- */

void Gate3::setSampleRate(int sr) { m_sampleRate = qMax(8000, sr); }
void Gate3::setThreshold(double lowDb, double highDb)
{
    m_lowThreshDb = lowDb;
    m_highThreshDb = highDb;
}
void Gate3::setAttack(double ms) { m_attackMs = qMax(0.1, ms); }
void Gate3::setRelease(double ms) { m_releaseMs = qMax(1.0, ms); }
void Gate3::setHold(double ms) { m_holdMs = qMax(0.0, ms); }
void Gate3::setRatio(double r) { m_ratio = qMax(1.0, r); }

/* ---- Simplified FFT magnitude spectrum ---- */

void Gate3::fftSpectrum(const QVector<double>& frame,
                          QVector<double>& magnitude) const
{
    int N = frame.size();
    magnitude.resize(N / 2);
    for (int k = 0; k < N / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im) / N;
    }
}

/* ---- Compute adaptive threshold from noise profile ---- */

double Gate3::adaptiveThreshold(const QVector<double>& spectrum,
                                  int bandStart, int bandEnd) const
{
    if (!m_noiseLearned) return 1e-6;
    double noiseEnergy = 0.0;
    int count = 0;
    for (int k = bandStart; k < qMin(bandEnd, spectrum.size()); ++k) {
        if (k < m_noiseSpectrum.size())
            noiseEnergy += m_noiseSpectrum[k];
        count++;
    }
    /* Threshold = noise floor + 6dB headroom */
    double avgNoise = (count > 0) ? noiseEnergy / count : 1e-6;
    return avgNoise * 2.0; /* ~6dB above noise floor */
}

/* ---- Compute gain from envelope and threshold ---- */

double Gate3::computeGain(double envDb, double threshDb)
{
    if (envDb >= threshDb) return 1.0;
    double belowDb = threshDb - envDb;
    double gainDb = -belowDb * (m_ratio - 1.0) / m_ratio;
    return qPow(10.0, gainDb / 20.0);
}

/* ---- Learn noise profile ---- */

void Gate3::learnNoiseProfile(const QVector<double>& frame)
{
    QVector<double> mag;
    fftSpectrum(frame, mag);

    if (m_noiseSpectrum.size() != mag.size()) {
        m_noiseSpectrum = mag;
    } else {
        /* Exponential moving average */
        double alpha = 0.1;
        for (int k = 0; k < mag.size(); ++k)
            m_noiseSpectrum[k] = m_noiseSpectrum[k] * (1.0 - alpha) + mag[k] * alpha;
    }
    m_noiseLearned = true;
}

/* ---- Process frame with dual-band gating ---- */

QVector<double> Gate3::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    int N = frame.size();
    if (N == 0) return {};

    /* Split frame into low and high bands via spectral processing */
    QVector<double> mag;
    fftSpectrum(frame, mag);
    int halfN = mag.size();

    /* Low band: 0 to N/4, High band: N/4 to N/2 */
    int splitBin = halfN / 2;

    /* Compute band energies */
    double lowEnergy = 0.0, highEnergy = 0.0;
    for (int k = 0; k < splitBin && k < mag.size(); ++k)
        lowEnergy += mag[k] * mag[k];
    for (int k = splitBin; k < mag.size(); ++k)
        highEnergy += mag[k] * mag[k];

    lowEnergy = qSqrt(lowEnergy / qMax(1, splitBin));
    highEnergy = qSqrt(highEnergy / qMax(1, halfN - splitBin));

    double lowDb = 20.0 * qLn(qMax(1e-10, lowEnergy)) / qLn(10.0);
    double highDb = 20.0 * qLn(qMax(1e-10, highEnergy)) / qLn(10.0);

    m_lastBands = {lowEnergy, highEnergy};

    /* Envelope followers with attack/release */
    double attackCoeff = qExp(-1.0 / (m_attackMs * m_sampleRate / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * m_sampleRate / 1000.0));

    m_envLow = (lowDb > m_envLow) ?
        attackCoeff * m_envLow + (1.0 - attackCoeff) * lowDb :
        releaseCoeff * m_envLow + (1.0 - releaseCoeff) * lowDb;
    m_envHigh = (highDb > m_envHigh) ?
        attackCoeff * m_envHigh + (1.0 - attackCoeff) * highDb :
        releaseCoeff * m_envHigh + (1.0 - releaseCoeff) * highDb;

    /* Adaptive thresholds from noise profile */
    double adaptLow = adaptiveThreshold(mag, 0, splitBin);
    double adaptHigh = adaptiveThreshold(mag, splitBin, halfN);
    double adaptLowDb = 20.0 * qLn(qMax(1e-10, adaptLow)) / qLn(10.0);
    double adaptHighDb = 20.0 * qLn(qMax(1e-10, adaptHigh)) / qLn(10.0);

    /* Use max of fixed and adaptive threshold */
    double threshLowDb = qMax(m_lowThreshDb, adaptLowDb);
    double threshHighDb = qMax(m_highThreshDb, adaptHighDb);

    /* Compute gains */
    double targetGainLow = computeGain(m_envLow, threshLowDb);
    double targetGainHigh = computeGain(m_envHigh, threshHighDb);

    /* Smooth gain transitions */
    double gCoeff = (targetGainLow < m_gainLow) ? releaseCoeff : attackCoeff;
    m_gainLow = gCoeff * m_gainLow + (1.0 - gCoeff) * targetGainLow;
    gCoeff = (targetGainHigh < m_gainHigh) ? releaseCoeff : attackCoeff;
    m_gainHigh = gCoeff * m_gainHigh + (1.0 - gCoeff) * targetGainHigh;

    /* Apply combined gain to output */
    double combinedGain = qMin(m_gainLow, m_gainHigh);
    QVector<double> output(N);
    for (int i = 0; i < N; ++i)
        output[i] = frame[i] * combinedGain;

    m_stats.totalFrames++;
    m_stats.frameSize = N;
    m_stats.sampleRate = m_sampleRate;
    m_stats.lowThreshold = threshLowDb;
    m_stats.highThreshold = threshHighDb;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    double reduction = 20.0 * qLn(qMax(1e-10, combinedGain)) / qLn(10.0);
    emit frameProcessed(N, reduction);
    return output;
}

QPair<double, double> Gate3::bandEnergies() const { return m_lastBands; }

void Gate3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
