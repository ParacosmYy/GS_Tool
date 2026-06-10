/**
 * @file Goertzel11.cpp
 * @brief Goertzel11 实现
 *
 * 实现Goertzel算法：广义双音检测与滑动窗能量累积的多频DTMF分析。
 */

#include "utils/fft277/Goertzel11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

constexpr double Goertzel11::DTMF_ROW[4];
constexpr double Goertzel11::DTMF_COL[4];
constexpr char Goertzel11::DTMF_MAP[4][4];

/* ---- Construction / Destruction ---- */

Goertzel11::Goertzel11(QObject *parent)
    : QObject(parent) {}

Goertzel11::~Goertzel11() = default;

/* ---- Configuration ---- */

void Goertzel11::setBlockSize(int n) { m_blockSize = qBound(16, n, 65536); }
void Goertzel11::setSampleRate(double rate) { m_sampleRate = qBound(1000.0, rate, 192000.0); }
void Goertzel11::setThreshold(double threshDb) { m_thresholdDb = threshDb; }

/* ---- Core Goertzel computation ---- */

void Goertzel11::goertzelCore(const QVector<double>& samples, double targetFreq,
                                double& real, double& imag, double& energy) const
{
    int n = qMin(samples.size(), m_blockSize);
    double k = 0.5 + n * targetFreq / m_sampleRate;
    double w = 2.0 * M_PI * k / n;
    double coeff = 2.0 * qCos(w);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < n; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    // Compute output: X[k] = s1 - s2 * e^{-jw}
    real = s1 - s2 * qCos(w);
    imag = s2 * qSin(w);
    energy = real * real + imag * imag;
}

/* ---- Sliding window energy ---- */

double Goertzel11::slidingEnergy(const QVector<double>& buffer, double targetFreq,
                                   int start, int length) const
{
    int end = qMin(start + length, buffer.size());
    QVector<double> segment(end - start);
    for (int i = start; i < end; ++i) segment[i - start] = buffer[i];

    double real, imag, energy;
    goertzelCore(segment, targetFreq, real, imag, energy);
    return energy;
}

/* ---- Detect single tone ---- */

Goertzel11::ToneResult Goertzel11::detectTone(const QVector<double>& samples, double targetFreq)
{
    QElapsedTimer timer;
    timer.start();

    ToneResult result;
    result.frequency = targetFreq;

    double real, imag, energy;
    goertzelCore(samples, targetFreq, real, imag, energy);

    result.magnitude = qSqrt(energy) / m_blockSize;
    result.energy = energy;
    result.phase = qAtan2(imag, real);

    double magDb = (result.magnitude > 1e-15) ? 20.0 * qLn(result.magnitude) / M_LN10 : -200.0;
    result.detected = magDb > m_thresholdDb;

    double elapsed = timer.elapsed();
    m_stats.numTones++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit toneDetected(targetFreq, result.magnitude, energy, elapsed);

    return result;
}

/* ---- Detect multiple tones ---- */

QVector<Goertzel11::ToneResult>
Goertzel11::detectMultiTone(const QVector<double>& samples,
                              const QVector<double>& targetFreqs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<ToneResult> results;
    for (double freq : targetFreqs) {
        ToneResult r;
        r.frequency = freq;
        double real, imag, energy;
        goertzelCore(samples, freq, real, imag, energy);
        r.magnitude = qSqrt(energy) / m_blockSize;
        r.energy = energy;
        r.phase = qAtan2(imag, real);
        double magDb = (r.magnitude > 1e-15) ? 20.0 * qLn(r.magnitude) / M_LN10 : -200.0;
        r.detected = magDb > m_thresholdDb;
        results.append(r);
    }

    double elapsed = timer.elapsed();
    m_stats.numTones += targetFreqs.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return results;
}

/* ---- Detect DTMF ---- */

Goertzel11::DTMFResult Goertzel11::detectDTMF(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    DTMFResult result;
    result.valid = false;

    // Detect row group (low frequencies)
    double maxRowEnergy = 0.0;
    int maxRowIdx = -1;
    QVector<double> rowEnergies(4);
    for (int i = 0; i < 4; ++i) {
        double real, imag, energy;
        goertzelCore(samples, DTMF_ROW[i], real, imag, energy);
        rowEnergies[i] = energy;
        if (energy > maxRowEnergy) { maxRowEnergy = energy; maxRowIdx = i; }
    }

    // Detect column group (high frequencies)
    double maxColEnergy = 0.0;
    int maxColIdx = -1;
    QVector<double> colEnergies(4);
    for (int i = 0; i < 4; ++i) {
        double real, imag, energy;
        goertzelCore(samples, DTMF_COL[i], real, imag, energy);
        colEnergies[i] = energy;
        if (energy > maxColEnergy) { maxColEnergy = energy; maxColIdx = i; }
    }

    // Compute total energy for SNR
    double totalRowEnergy = 0.0, totalColEnergy = 0.0;
    for (int i = 0; i < 4; ++i) {
        totalRowEnergy += rowEnergies[i];
        totalColEnergy += colEnergies[i];
    }

    if (maxRowIdx >= 0 && maxColIdx >= 0) {
        result.rowFreq = DTMF_ROW[maxRowIdx];
        result.colFreq = DTMF_COL[maxColIdx];
        result.rowMag = qSqrt(maxRowEnergy) / m_blockSize;
        result.colMag = qSqrt(maxColEnergy) / m_blockSize;
        result.digit = DTMF_MAP[maxRowIdx][maxColIdx];

        // SNR: ratio of peak energy to sum of others
        double otherRow = totalRowEnergy - maxRowEnergy;
        double otherCol = totalColEnergy - maxColEnergy;
        double snrRow = (otherRow > 0) ? 10.0 * qLn(maxRowEnergy / otherRow) / M_LN10 : 40.0;
        double snrCol = (otherCol > 0) ? 10.0 * qLn(maxColEnergy / otherCol) / M_LN10 : 40.0;
        result.snr = qMin(snrRow, snrCol);

        // Validate: SNR > 10 dB and both tones above threshold
        double rowDb = (result.rowMag > 1e-15) ? 20.0 * qLn(result.rowMag) / M_LN10 : -200.0;
        double colDb = (result.colMag > 1e-15) ? 20.0 * qLn(result.colMag) / M_LN10 : -200.0;
        result.valid = (result.snr > 10.0) && (rowDb > m_thresholdDb) && (colDb > m_thresholdDb);
    }

    double elapsed = timer.elapsed();
    if (result.valid) m_stats.numDTMFDetected++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit dtmfDetected(result.digit, result.rowFreq, result.colFreq, elapsed);

    return result;
}

/* ---- Sliding window DTMF ---- */

QVector<Goertzel11::DTMFResult>
Goertzel11::slidingDTMF(const QVector<double>& buffer, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<DTMFResult> results;
    hopSize = qBound(1, hopSize, m_blockSize);

    for (int pos = 0; pos + m_blockSize <= buffer.size(); pos += hopSize) {
        QVector<double> block(m_blockSize);
        for (int i = 0; i < m_blockSize; ++i) block[i] = buffer[pos + i];
        DTMFResult r = detectDTMF(block);
        results.append(r);
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return results;
}

/* ---- Reset ---- */

void Goertzel11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
