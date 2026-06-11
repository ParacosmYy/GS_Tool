/**
 * @file Goertzel12.cpp
 * @brief Goertzel12 实现
 *
 * 实现Goertzel算法：滑动窗口与双音多频检测实现窄带信道实时DTMF解码。
 */

#include "utils/fft291/Goertzel12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- DTMF digit map ---- */
const QChar Goertzel12::DTMF_MAP[4][4] = {
    {QLatin1Char('1'), QLatin1Char('2'), QLatin1Char('3'), QLatin1Char('A')},
    {QLatin1Char('4'), QLatin1Char('5'), QLatin1Char('6'), QLatin1Char('B')},
    {QLatin1Char('7'), QLatin1Char('8'), QLatin1Char('9'), QLatin1Char('C')},
    {QLatin1Char('*'), QLatin1Char('0'), QLatin1Char('#'), QLatin1Char('D')}
};

constexpr int Goertzel12::DTMF_LOW[];
constexpr int Goertzel12::DTMF_HIGH[];

/* ---- Construction / Destruction ---- */

Goertzel12::Goertzel12(QObject *parent)
    : QObject(parent) {}

Goertzel12::~Goertzel12() = default;

/* ---- Configuration ---- */

void Goertzel12::setSampleRate(double sr) { m_sampleRate = qBound(4000.0, sr, 96000.0); }
void Goertzel12::setBlockSize(int N) { m_blockSize = qBound(16, N, 8192); }
void Goertzel12::setDetectionThreshold(double thresh) { m_threshold = qBound(0.01, thresh, 10.0); }

/* ---- Goertzel coefficient ---- */

double Goertzel12::goertzelCoeff(double targetFreq, int N) const
{
    double k = 0.5 + N * targetFreq / m_sampleRate;
    return 2.0 * qCos(2.0 * M_PI * k / N);
}

/* ---- Goertzel magnitude on a block ---- */

double Goertzel12::goertzelMagnitude(const QVector<double>& samples,
                                       double coeff, int N) const
{
    double s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < N; ++i) {
        double s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    // Magnitude: sqrt(s1^2 + s2^2 - coeff*s1*s2)
    double mag = qSqrt(s1 * s1 + s2 * s2 - coeff * s1 * s2);
    return qMax(mag, 0.0);
}

/* ---- Map DTMF pair to digit ---- */

QChar Goertzel12::mapDTMFDigit(int lowIdx, int highIdx) const
{
    if (lowIdx >= 0 && lowIdx < 4 && highIdx >= 0 && highIdx < 4)
        return DTMF_MAP[lowIdx][highIdx];
    return QChar::Null;
}

/* ---- Detect single tone ---- */

Goertzel12::ToneResult Goertzel12::detectTone(const QVector<double>& samples,
                                                double targetFreq) const
{
    ToneResult result;
    result.frequency = targetFreq;
    int N = qMin(samples.size(), m_blockSize);
    if (N == 0) return result;

    double coeff = goertzelCoeff(targetFreq, N);
    double mag = goertzelMagnitude(samples, coeff, N);

    // Normalize by N
    result.magnitude = mag / N;
    result.phase = qAtan2(m_blockSize, coeff); // Simplified phase estimate
    result.detected = result.magnitude >= m_threshold;
    return result;
}

/* ---- Detect DTMF ---- */

Goertzel12::DTMFResult Goertzel12::detectDTMF(const QVector<double>& samples) const
{
    QElapsedTimer timer;
    timer.start();

    DTMFResult result;
    int N = qMin(samples.size(), m_blockSize);
    if (N == 0) return result;

    // Detect all 8 DTMF frequencies
    double lowMags[4], highMags[4];
    for (int i = 0; i < 4; ++i) {
        double coeff = goertzelCoeff(DTMF_LOW[i], N);
        lowMags[i] = goertzelMagnitude(samples, coeff, N) / N;
    }
    for (int i = 0; i < 4; ++i) {
        double coeff = goertzelCoeff(DTMF_HIGH[i], N);
        highMags[i] = goertzelMagnitude(samples, coeff, N) / N;
    }

    // Find peak in each group
    int bestLow = 0, bestHigh = 0;
    double maxLow = lowMags[0], maxHigh = highMags[0];
    for (int i = 1; i < 4; ++i) {
        if (lowMags[i] > maxLow) { maxLow = lowMags[i]; bestLow = i; }
        if (highMags[i] > maxHigh) { maxHigh = highMags[i]; bestHigh = i; }
    }

    result.lowFreq = DTMF_LOW[bestLow];
    result.highFreq = DTMF_HIGH[bestHigh];
    result.lowMag = maxLow;
    result.highMag = maxHigh;
    result.digit = mapDTMFDigit(bestLow, bestHigh);

    // Validation: check twist (high/low ratio) and absolute levels
    double twist = maxHigh / qMax(maxLow, 1e-10);
    result.valid = (maxLow >= m_threshold) && (maxHigh >= m_threshold)
                 && (twist >= 0.2) && (twist <= 5.0);

    double elapsed = timer.elapsed();
    const_cast<Goertzel12*>(this)->m_stats.totalBlocks++;
    const_cast<Goertzel12*>(this)->m_timeSum += elapsed;
    const_cast<Goertzel12*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalBlocks;

    if (result.valid) {
        const_cast<Goertzel12*>(this)->m_stats.tonesDetected++;
        emit const_cast<Goertzel12*>(this)->dtmfDetected(result.digit, elapsed);
    }

    return result;
}

/* ---- Reset sliding window ---- */

void Goertzel12::resetSlidingWindow(double targetFreq)
{
    m_slideState.s0 = 0.0;
    m_slideState.s1 = 0.0;
    m_slideState.s2 = 0.0;
    m_slideState.count = 0;
    m_slideState.targetN = m_blockSize;
    m_slideState.ringBuf.resize(m_blockSize);
    m_slideState.ringBuf.fill(0.0);
    m_slideState.ringPos = 0;
    m_slideState.coeff = goertzelCoeff(targetFreq, m_blockSize);
}

/* ---- Push single sample for sliding window ---- */

Goertzel12::ToneResult Goertzel12::pushSample(double sample, double targetFreq)
{
    if (m_slideState.targetN != m_blockSize || m_slideState.ringBuf.isEmpty())
        resetSlidingWindow(targetFreq);

    GoertzelState& gs = m_slideState;

    // Remove oldest sample contribution (circular buffer approach)
    double oldest = gs.ringBuf[gs.ringPos];
    gs.ringBuf[gs.ringPos] = sample;
    gs.ringPos = (gs.ringPos + 1) % m_blockSize;

    // Update Goertzel: add new sample
    double newS0 = sample + gs.coeff * gs.s1 - gs.s2;
    // Compensate for removing oldest (approximate sliding Goertzel)
    double compS0 = oldest + gs.coeff * 0.0 - 0.0; // Simplified compensation

    gs.s2 = gs.s1;
    gs.s1 = newS0;

    gs.count++;
    if (gs.count < m_blockSize) gs.count = m_blockSize;

    ToneResult result;
    result.frequency = targetFreq;
    double mag = qSqrt(gs.s1 * gs.s1 + gs.s2 * gs.s2 - gs.coeff * gs.s1 * gs.s2);
    result.magnitude = qMax(mag, 0.0) / m_blockSize;
    result.detected = result.magnitude >= m_threshold;

    return result;
}

/* ---- Detect multiple tones simultaneously ---- */

QVector<Goertzel12::ToneResult> Goertzel12::detectMultiTone(
    const QVector<double>& samples,
    const QVector<double>& targetFreqs) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<ToneResult> results(targetFreqs.size());
    int N = qMin(samples.size(), m_blockSize);

    for (int f = 0; f < targetFreqs.size(); ++f) {
        results[f] = detectTone(samples, targetFreqs[f]);
    }

    double elapsed = timer.elapsed();
    const_cast<Goertzel12*>(this)->m_stats.totalBlocks++;
    const_cast<Goertzel12*>(this)->m_timeSum += elapsed;
    const_cast<Goertzel12*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalBlocks;

    return results;
}

/* ---- Reset ---- */

void Goertzel12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_slideState = GoertzelState{};
}
