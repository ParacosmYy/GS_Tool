/**
 * @file PitchDetector5.cpp
 * @brief PitchDetector5 实现
 *
 * 实现基频检测：归一化互相关(NCC)、YIN累积均值差函数(CMDF)、抛物线精化。
 */

#include "utils/signal218/PitchDetector5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PitchDetector5::PitchDetector5(QObject *parent) : QObject(parent)
{
    updateLagBounds();
}

PitchDetector5::~PitchDetector5() = default;

/* ---- Configuration ---- */

void PitchDetector5::setParameters(int sampleRate, int frameSize,
                                     double minFreq, double maxFreq)
{
    m_sampleRate = qMax(8000, sampleRate);
    m_frameSize = qMax(64, frameSize);
    m_minFreq = qMax(20.0, minFreq);
    m_maxFreq = qMin(static_cast<double>(m_sampleRate / 2), maxFreq);
    updateLagBounds();

    m_stats.sampleRate = m_sampleRate;
    m_stats.frameSize = m_frameSize;
    m_stats.minFreq = m_minFreq;
    m_stats.maxFreq = m_maxFreq;
}

/* ---- Update lag bounds ---- */

void PitchDetector5::updateLagBounds()
{
    m_maxLag = qMin(m_frameSize / 2,
                     static_cast<int>(m_sampleRate / m_minFreq));
    m_minLag = qMax(2, static_cast<int>(m_sampleRate / m_maxFreq));
}

/* ---- Normalized cross-correlation ---- */

QVector<double> PitchDetector5::ncc(const QVector<double>& frame) const
{
    int n = frame.size();
    int maxL = qMin(m_maxLag, n / 2);
    QVector<double> nccVals(maxL + 1, 0.0);

    // Compute signal energy
    double energy = 0.0;
    for (int i = 0; i < n; ++i) energy += frame[i] * frame[i];
    if (energy < 1e-20) return nccVals;

    for (int lag = m_minLag; lag <= maxL; ++lag) {
        double num = 0.0;
        double denL = 0.0, denR = 0.0;
        for (int i = 0; i < n - lag; ++i) {
            num += frame[i] * frame[i + lag];
            denL += frame[i] * frame[i];
            denR += frame[i + lag] * frame[i + lag];
        }
        double den = qSqrt(denL * denR);
        nccVals[lag] = (den > 1e-20) ? num / den : 0.0;
    }
    return nccVals;
}

/* ---- YIN cumulative mean difference function ---- */

QVector<double> PitchDetector5::cmdf(const QVector<double>& frame) const
{
    int n = frame.size();
    int maxL = qMin(m_maxLag, n / 2);

    // Step 1: Difference function
    QVector<double> diff(maxL + 1, 0.0);
    for (int lag = 0; lag <= maxL; ++lag) {
        for (int i = 0; i < n - lag; ++i) {
            double d = frame[i] - frame[i + lag];
            diff[lag] += d * d;
        }
    }

    // Step 2: Cumulative mean normalization
    QVector<double> cmdfVals(maxL + 1, 0.0);
    cmdfVals[0] = 1.0;
    double cumSum = 0.0;
    for (int lag = 1; lag <= maxL; ++lag) {
        cumSum += diff[lag];
        cmdfVals[lag] = (cumSum > 1e-20)
            ? diff[lag] * lag / cumSum : 1.0;
    }
    return cmdfVals;
}

/* ---- Parabolic refinement ---- */

double PitchDetector5::parabolicRefine(const QVector<double>& func, int idx) const
{
    if (idx <= 0 || idx >= func.size() - 1) return static_cast<double>(idx);
    double y0 = func[idx - 1];
    double y1 = func[idx];
    double y2 = func[idx + 1];
    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-15) return static_cast<double>(idx);
    double delta = (y0 - y2) / denom;
    return static_cast<double>(idx) + qBound(-0.5, delta, 0.5);
}

/* ---- Detect using NCC ---- */

PitchDetector5::PitchResult PitchDetector5::detectNCC(const QVector<double>& frame) const
{
    PitchResult result;
    QVector<double> nccVals = ncc(frame);

    // Find peak in NCC
    double bestVal = -1.0;
    int bestLag = m_minLag;
    for (int lag = m_minLag; lag < nccVals.size(); ++lag) {
        if (nccVals[lag] > bestVal) {
            bestVal = nccVals[lag];
            bestLag = lag;
        }
    }

    if (bestVal > 0.5) {
        double refined = parabolicRefine(nccVals, bestLag);
        result.period = bestLag;
        result.frequency = m_sampleRate / qMax(refined, 1.0);
        result.confidence = qBound(0.0, bestVal, 1.0);
        result.voiced = true;
    } else {
        result.voiced = false;
        result.confidence = 0.0;
    }
    return result;
}

/* ---- Detect using YIN ---- */

PitchDetector5::PitchResult PitchDetector5::detectYIN(const QVector<double>& frame) const
{
    PitchResult result;
    QVector<double> cmdfVals = cmdf(frame);

    // Absolute threshold
    const double threshold = 0.15;

    // Find first dip below threshold
    int bestLag = -1;
    for (int lag = m_minLag; lag < cmdfVals.size(); ++lag) {
        if (cmdfVals[lag] < threshold) {
            // Find local minimum in this valley
            bestLag = lag;
            while (bestLag + 1 < cmdfVals.size() &&
                   cmdfVals[bestLag + 1] < cmdfVals[bestLag])
                bestLag++;
            break;
        }
    }

    if (bestLag >= m_minLag) {
        double refined = parabolicRefine(cmdfVals, bestLag);
        result.period = bestLag;
        result.frequency = m_sampleRate / qMax(refined, 1.0);
        result.confidence = 1.0 - qMin(cmdfVals[bestLag], 1.0);
        result.voiced = true;
    } else {
        result.voiced = false;
        result.confidence = 0.0;
    }
    return result;
}

/* ---- Combined detection ---- */

PitchDetector5::PitchResult PitchDetector5::detect(const QVector<double>& frame) const
{
    QElapsedTimer timer;
    timer.start();

    // Run both NCC and YIN, pick higher confidence
    PitchResult nccResult = detectNCC(frame);
    PitchResult yinResult = detectYIN(frame);

    PitchResult best = (nccResult.confidence >= yinResult.confidence)
        ? nccResult : yinResult;

    const_cast<PitchDetector5*>(this)->m_stats.totalFrames++;
    const_cast<PitchDetector5*>(this)->m_timeSum += timer.elapsed();
    const_cast<PitchDetector5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalFrames;

    emit pitchDetected(best.frequency, best.confidence, timer.elapsed());
    return best;
}

/* ---- Reset ---- */

void PitchDetector5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
