/**
 * @file Goertzel9.cpp
 * @brief Goertzel9 实现
 *
 * 实现Goertzel算法：多音并行检测与可配置跳距滑动窗口。
 */

#include "utils/fft249/Goertzel9.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Goertzel9::Goertzel9(QObject *parent) : QObject(parent) { initRing(); }
Goertzel9::~Goertzel9() = default;

/* ---- Configuration ---- */

void Goertzel9::setTargetFreqs(const QVector<double>& freqs, double sampleRate, int blockSize)
{
    m_sampleRate = sampleRate;
    m_blockSize = qMax(1, blockSize);
    m_tones.clear();
    m_states.clear();

    for (double f : freqs) {
        m_tones.append(computeCoeffs(f, sampleRate, m_blockSize));
        m_states.append({});
    }
    m_stats.numTones = m_tones.size();
    initRing();
}

void Goertzel9::setThresholdDb(double threshold) { m_thresholdDb = threshold; }
void Goertzel9::setHopSize(int hop) { m_hopSize = qMax(1, hop); }

/* ---- Compute Goertzel coefficients ---- */

Goertzel9::ToneCoeffs Goertzel9::computeCoeffs(double freq, double sampleRate, int N) const
{
    double k = 0.5 + N * freq / sampleRate;
    double w = 2.0 * M_PI * k / N;
    double cosine = qCos(w);
    double sine = qSin(w);
    double coeff = 2.0 * cosine;
    return {freq, coeff, coeff * coeff / 2.0 - 1.0, sine, cosine};
}

/* ---- Single Goertzel on a block ---- */

Goertzel9::ToneResult Goertzel9::goertzelSingle(
    const QVector<double>& block, const ToneCoeffs& tc) const
{
    double s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < block.size(); ++i) {
        double s0 = block[i] + tc.coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return extractResult(s1, s2, tc);
}

/* ---- Extract result from final state ---- */

Goertzel9::ToneResult Goertzel9::extractResult(
    double s1, double s2, const ToneCoeffs& tc) const
{
    ToneResult r;
    r.frequency = tc.freq;
    // Real and imaginary parts
    double real = s1 - s2 * tc.cosineVal;
    double imag = s2 * tc.sineVal;
    r.magnitude = qSqrt(real * real + imag * imag);
    r.magnitudeDb = (r.magnitude > 1e-20)
        ? 20.0 * qLn(r.magnitude) / qLn(10.0) : -200.0;
    r.phase = qAtan2(imag, real);
    r.detected = r.magnitudeDb >= m_thresholdDb;
    return r;
}

/* ---- Initialize ring buffer ---- */

void Goertzel9::initRing()
{
    m_ringBuf.resize(m_blockSize, 0.0);
    m_ringPos = 0;
    m_ringFill = 0;
    for (auto& s : m_states) s = {};
}

/* ---- Process a block ---- */

QVector<Goertzel9::ToneResult> Goertzel9::processBlock(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<ToneResult> results(m_tones.size());
    for (int t = 0; t < m_tones.size(); ++t)
        results[t] = goertzelSingle(samples, m_tones[t]);

    int detections = 0;
    for (const auto& r : results)
        if (r.detected) detections++;

    m_stats.blockSize = samples.size();
    m_stats.numDetections = detections;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tonesDetected(detections,
        results.isEmpty() ? -200.0 : results[0].magnitudeDb, timer.elapsed());
    return results;
}

/* ---- Process streaming (sliding window) ---- */

QVector<Goertzel9::ToneResult> Goertzel9::processStreaming(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<ToneResult> results;

    for (int i = 0; i < samples.size(); ++i) {
        // Push sample into ring buffer
        double oldSample = m_ringBuf[m_ringPos];
        m_ringBuf[m_ringPos] = samples[i];
        m_ringPos = (m_ringPos + 1) % m_blockSize;
        m_ringFill = qMin(m_ringFill + 1, m_blockSize);

        // Check if we've reached a hop boundary
        if (m_ringFill >= m_blockSize && ((i + 1) % m_hopSize == 0)) {
            // Run Goertzel on current window
            for (int t = 0; t < m_tones.size(); ++t) {
                results.append(goertzelSingle(m_ringBuf, m_tones[t]));
            }
        }
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return results;
}

/* ---- DTMF detection ---- */

QVector<Goertzel9::ToneResult> Goertzel9::detectDTMF(const QVector<double>& samples)
{
    // Standard DTMF frequencies
    QVector<double> dtmfFreqs = {
        697.0, 770.0, 852.0, 941.0,   // Row tones
        1209.0, 1336.0, 1477.0, 1633.0 // Col tones
    };

    // Temporarily set DTMF frequencies
    QVector<ToneCoeffs> savedTones = m_tones;
    m_tones.clear();
    for (double f : dtmfFreqs)
        m_tones.append(computeCoeffs(f, m_sampleRate, m_blockSize));

    auto results = processBlock(samples);

    // Restore original tones
    m_tones = savedTones;

    return results;
}

/* ---- Reset ---- */

void Goertzel9::resetStatistics()
{
    m_tones.clear();
    m_states.clear();
    initRing();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
