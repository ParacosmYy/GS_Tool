/**
 * @file Goertzel6.cpp
 * @brief Goertzel6 实现
 *
 * 实现Goertzel算法：并行多音检测、能量DTMF验证、实时频率分析。
 */

#include "utils/fft206/Goertzel6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Static DTMF tables ---- */

constexpr double Goertzel6::DTMF_LOW[4];
constexpr double Goertzel6::DTMF_HIGH[4];
constexpr char Goertzel6::DTMF_MAP[4][4];

/* ---- Construction / Destruction ---- */

Goertzel6::Goertzel6(QObject *parent) : QObject(parent) {}
Goertzel6::~Goertzel6() = default;

/* ---- Configuration ---- */

void Goertzel6::setSampleRate(int rate) { m_sampleRate = qMax(1, rate); }
void Goertzel6::setBlockSize(int N) { m_blockSize = qMax(1, N); }

/* ---- Single frequency Goertzel ---- */

double Goertzel6::goertzelMag(const QVector<double>& samples, double targetFreq) const
{
    int N = samples.size();
    if (N == 0) return 0.0;

    double k = qRound(targetFreq * N / m_sampleRate);
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    // Compute magnitude squared
    double mag2 = s1 * s1 + s2 * s2 - coeff * s1 * s2;
    return qSqrt(qFabs(mag2));
}

/* ---- Multi-tone detection ---- */

QVector<QPair<double, double>> Goertzel6::detectMultiTone(
    const QVector<double>& samples, const QVector<double>& frequencies) const
{
    QVector<QPair<double, double>> results;
    results.reserve(frequencies.size());

    // Parallel detection: run Goertzel for each frequency independently
    for (double freq : frequencies) {
        double mag = goertzelMag(samples, freq);
        results.append({freq, mag});
    }

    return results;
}

/* ---- DTMF detection ---- */

Goertzel6::DtmfResult Goertzel6::detectDTMF(const QVector<double>& samples) const
{
    QElapsedTimer timer;
    timer.start();
    DtmfResult result;

    int N = samples.size();
    if (N == 0) { result.valid = false; return result; }

    // Compute total energy for validation
    double totalEnergy = 0.0;
    for (int i = 0; i < N; ++i)
        totalEnergy += samples[i] * samples[i];

    // Detect low-group frequencies
    double maxLowEnergy = 0.0;
    int bestLow = -1;
    for (int i = 0; i < 4; ++i) {
        double mag = goertzelMag(samples, DTMF_LOW[i]);
        double energy = mag * mag;
        if (energy > maxLowEnergy) { maxLowEnergy = energy; bestLow = i; }
    }

    // Detect high-group frequencies
    double maxHighEnergy = 0.0;
    int bestHigh = -1;
    for (int i = 0; i < 4; ++i) {
        double mag = goertzelMag(samples, DTMF_HIGH[i]);
        double energy = mag * mag;
        if (energy > maxHighEnergy) { maxHighEnergy = energy; bestHigh = i; }
    }

    result.lowFreqEnergy = maxLowEnergy;
    result.highFreqEnergy = maxHighEnergy;

    // Validate with twist threshold
    result.valid = validateDTMF(maxLowEnergy, maxHighEnergy, totalEnergy, 6.0);

    if (result.valid && bestLow >= 0 && bestHigh >= 0)
        result.digit = DTMF_MAP[bestLow][bestHigh];

    return result;
}

/* ---- DTMF validation ---- */

bool Goertzel6::validateDTMF(double lowEnergy, double highEnergy,
                               double totalEnergy, double twistThreshold) const
{
    // Minimum energy threshold
    if (totalEnergy < 1e-10) return false;

    // Both tones must have significant energy relative to total
    double lowRatio = lowEnergy / totalEnergy;
    double highRatio = highEnergy / totalEnergy;
    if (lowRatio < 0.1 || highRatio < 0.1) return false;

    // Twist check: difference between low and high should not be too large
    double twist = 10.0 * qLn(lowEnergy / qMax(highEnergy, 1e-20)) / qLn(10.0);
    if (qFabs(twist) > twistThreshold) return false;

    return true;
}

/* ---- Reset ---- */

void Goertzel6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
