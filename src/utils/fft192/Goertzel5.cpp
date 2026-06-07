/**
 * @file Goertzel5.cpp
 * @brief Goertzel5 实现
 *
 * 实现Goertzel算法：滑动窗口实时音调检测、DTMF解码。
 */

#include "utils/fft192/Goertzel5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- DTMF constant definitions ---- */
constexpr double Goertzel5::DTMF_ROWS[4];
constexpr double Goertzel5::DTMF_COLS[4];
constexpr char Goertzel5::DTMF_MAP[4][4];

/* ---- Construction / Destruction ---- */

Goertzel5::Goertzel5(QObject *parent) : QObject(parent) {}
Goertzel5::~Goertzel5() = default;

/* ---- Configuration ---- */

void Goertzel5::setSampleRate(int sr) { m_sampleRate = qMax(4000, sr); }
void Goertzel5::setWindowSize(int N) { m_windowSize = qMax(32, N); }

/* ---- Core Goertzel for one frequency ---- */

double Goertzel5::goertzel(const QVector<double>& data,
                            double targetFreq) const
{
    int N = data.size();
    if (N == 0) return 0.0;

    double k = static_cast<int>(0.5 + N * targetFreq / m_sampleRate);
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = data[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    // Compute magnitude squared
    double re = s1 - s2 * qCos(w);
    double im = s2 * qSin(w);
    return qSqrt(re * re + im * im);
}

/* ---- Find dominant index ---- */

int Goertzel5::findDominant(const QVector<double>& mags) const
{
    double best = 0.0;
    int idx = 0;
    for (int i = 0; i < mags.size(); ++i) {
        if (mags[i] > best) {
            best = mags[i];
            idx = i;
        }
    }
    return idx;
}

/* ---- Single frequency detection ---- */

double Goertzel5::detectFrequency(const QVector<double>& data,
                                    double targetFreq) const
{
    return goertzel(data, targetFreq);
}

/* ---- Multi-frequency detection ---- */

QVector<QPair<double, double>> Goertzel5::detectFrequencies(
    const QVector<double>& data,
    const QVector<double>& targets) const
{
    QVector<QPair<double, double>> results;
    results.reserve(targets.size());
    for (double f : targets)
        results.append({f, goertzel(data, f)});
    return results;
}

/* ---- Decode single DTMF tone ---- */

Goertzel5::DTMFResult Goertzel5::decodeDTMF(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    DTMFResult result;

    // Compute row magnitudes
    QVector<double> rowMags(4);
    for (int i = 0; i < 4; ++i)
        rowMags[i] = goertzel(data, DTMF_ROWS[i]);

    // Compute column magnitudes
    QVector<double> colMags(4);
    for (int i = 0; i < 4; ++i)
        colMags[i] = goertzel(data, DTMF_COLS[i]);

    int rowIdx = findDominant(rowMags);
    int colIdx = findDominant(colMags);

    result.rowFreq = DTMF_ROWS[rowIdx];
    result.colFreq = DTMF_COLS[colIdx];
    result.rowMag = rowMags[rowIdx];
    result.colMag = colMags[colIdx];
    result.digit = DTMF_MAP[rowIdx][colIdx];

    // Confidence: ratio of dominant to sum of all in same group
    double rowSum = 0.0;
    for (double m : rowMags) rowSum += m;
    double colSum = 0.0;
    for (double m : colMags) colSum += m;
    result.confidence = (rowSum > 0 && colSum > 0)
        ? (rowMags[rowIdx] / rowSum + colMags[colIdx] / colSum) * 0.5
        : 0.0;

    m_stats.totalDetections++;
    m_stats.windowSize = data.size();
    m_stats.sampleRate = m_sampleRate;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit dtmfDecoded(result.digit, result.confidence);
    return result;
}

/* ---- Decode DTMF sequence from audio ---- */

QString Goertzel5::decodeDTMFSequence(const QVector<double>& data, int frameStep)
{
    QElapsedTimer timer;
    timer.start();

    if (frameStep <= 0) frameStep = m_windowSize / 2;
    int N = data.size();
    QString result;
    QChar lastDigit;

    for (int pos = 0; pos + m_windowSize <= N; pos += frameStep) {
        QVector<double> frame(m_windowSize);
        for (int i = 0; i < m_windowSize; ++i)
            frame[i] = data[pos + i];

        auto dr = decodeDTMF(frame);

        // Only add digit if confidence is high enough and different from last
        if (dr.confidence > 0.55 && dr.digit != lastDigit) {
            result += dr.digit;
            lastDigit = dr.digit;
        }
    }

    m_stats.dtmfDigits = result.size();
    m_timeSum += timer.elapsed();
    emit detectionCompleted(0.0, 0.0, timer.elapsed());
    return result;
}

/* ---- Sliding window detection ---- */

QVector<double> Goertzel5::slidingDetect(const QVector<double>& samples,
                                           double targetFreq, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    if (hopSize <= 0) hopSize = m_windowSize / 4;
    int N = samples.size();
    int numFrames = (N - m_windowSize) / hopSize + 1;
    if (numFrames <= 0) numFrames = 1;

    QVector<double> magnitudes(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        int end = qMin(start + m_windowSize, N);
        QVector<double> frame(end - start);
        for (int i = start; i < end; ++i)
            frame[i - start] = samples[i];
        magnitudes[f] = goertzel(frame, targetFreq);
    }

    m_stats.totalDetections += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(targetFreq, magnitudes.isEmpty() ? 0.0 : magnitudes[0],
                            timer.elapsed());
    return magnitudes;
}

/* ---- Reset ---- */

void Goertzel5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
