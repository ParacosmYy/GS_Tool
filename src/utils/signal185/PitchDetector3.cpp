/**
 * @file PitchDetector3.cpp
 * @brief PitchDetector3 实现
 *
 * 实现基频检测：自相关函数、抛物线插值精化、有声/无声判决、倍频校正。
 */

#include "utils/signal185/PitchDetector3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PitchDetector3::PitchDetector3(int sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(sampleRate) {}
PitchDetector3::~PitchDetector3() = default;

/* ---- Configuration ---- */

void PitchDetector3::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); }
void PitchDetector3::setPitchRange(double minHz, double maxHz)
{
    m_minPitch = qMax(20.0, minHz);
    m_maxPitch = qMax(m_minPitch + 1.0, maxHz);
}
void PitchDetector3::setVoicingThreshold(double threshold) { m_voiceThreshold = qBound(0.0, threshold, 1.0); }
void PitchDetector3::setClarityThreshold(double threshold) { m_clarityThreshold = qBound(0.0, threshold, 1.0); }

/* ---- RMS energy ---- */

double PitchDetector3::rmsEnergy(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : frame) sum += s * s;
    return qSqrt(sum / frame.size());
}

/* ---- Zero crossing rate ---- */

double PitchDetector3::zeroCrossingRate(const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;
    int crossings = 0;
    for (int i = 1; i < frame.size(); ++i) {
        if ((frame[i] >= 0.0) != (frame[i - 1] >= 0.0)) crossings++;
    }
    return static_cast<double>(crossings) / (frame.size() - 1);
}

/* ---- Autocorrelation ---- */

QVector<double> PitchDetector3::autocorrelation(const QVector<double>& frame) const
{
    int N = frame.size();
    if (N == 0) return {};

    QVector<double> acf(N, 0.0);
    // Biased autocorrelation
    for (int lag = 0; lag < N; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; ++i)
            sum += frame[i] * frame[i + lag];
        acf[lag] = sum / N;
    }
    return acf;
}

/* ---- Find peak in range ---- */

int PitchDetector3::findPeakInRange(const QVector<double>& acf,
                                      int minLag, int maxLag) const
{
    if (minLag >= maxLag || minLag < 0 || maxLag > acf.size()) return -1;

    int peakIdx = minLag;
    double peakVal = acf[minLag];
    for (int i = minLag + 1; i < maxLag && i < acf.size(); ++i) {
        if (acf[i] > peakVal) { peakVal = acf[i]; peakIdx = i; }
    }

    // Verify it's a local maximum (not edge)
    if (peakIdx > 0 && peakIdx < acf.size() - 1) {
        if (acf[peakIdx] < acf[peakIdx - 1] && acf[peakIdx] < acf[peakIdx + 1])
            return -1; // Not a peak
    }
    return peakIdx;
}

/* ---- Parabolic interpolation ---- */

double PitchDetector3::parabolicInterpolation(const QVector<double>& acf,
                                                int peakIndex) const
{
    if (peakIndex <= 0 || peakIndex >= acf.size() - 1) return peakIndex;

    double alpha = acf[peakIndex - 1];
    double beta = acf[peakIndex];
    double gamma = acf[peakIndex + 1];

    double denom = 2.0 * (2.0 * beta - alpha - gamma);
    if (qAbs(denom) < 1e-12) return peakIndex;

    double offset = (alpha - gamma) / denom;
    return peakIndex + offset;
}

/* ---- Voicing decision ---- */

bool PitchDetector3::decideVoiced(const QVector<double>& acf,
                                    const QVector<double>& frame) const
{
    if (acf.size() < 2) return false;

    double energy = rmsEnergy(frame);
    if (energy < m_voiceThreshold * 0.01) return false;

    // Check ACF peak-to-valley ratio in pitch range
    double sr = static_cast<double>(m_sampleRate);
    int minLag = qMax(1, static_cast<int>(sr / m_maxPitch));
    int maxLag = qMin(acf.size() - 1, static_cast<int>(sr / m_minPitch));

    int peak = findPeakInRange(acf, minLag, maxLag);
    if (peak < 0) return false;

    // Voiced if peak ACF is above threshold relative to ACF[0]
    double clarity = (acf[0] > 1e-12) ? acf[peak] / acf[0] : 0.0;
    return clarity > m_voiceThreshold;
}

/* ---- Octave correction ---- */

double PitchDetector3::octaveCorrect(double freq, const QVector<double>& acf,
                                       double samplePeriod) const
{
    if (freq <= 0.0) return freq;

    // Check if half-frequency (double period) has higher correlation
    double halfFreq = freq / 2.0;
    if (halfFreq < m_minPitch) return freq;

    int periodLag = qMax(1, static_cast<int>(qRound(freq * samplePeriod)));
    int doubleLag = qMin(acf.size() - 1, 2 * periodLag);

    if (doubleLag < acf.size()) {
        double ratio = (acf[periodLag] > 1e-12) ? acf[doubleLag] / acf[periodLag] : 0.0;
        if (ratio > 0.9 && halfFreq >= m_minPitch) return halfFreq;
    }

    // Check if double frequency (half period) is better
    double doubleFreq = freq * 2.0;
    if (doubleFreq > m_maxPitch) return freq;

    int halfLag = qMax(1, periodLag / 2);
    if (halfLag < acf.size()) {
        double ratio = (acf[periodLag] > 1e-12) ? acf[halfLag] / acf[periodLag] : 0.0;
        if (ratio > 0.95) return doubleFreq;
    }

    return freq;
}

/* ---- Single frame detection ---- */

PitchDetector3::PitchResult PitchDetector3::detect(const QVector<double>& frame) const
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;
    if (frame.isEmpty()) return result;

    QVector<double> acf = autocorrelation(frame);

    double sr = static_cast<double>(m_sampleRate);
    int minLag = qMax(1, static_cast<int>(sr / m_maxPitch));
    int maxLag = qMin(acf.size() - 1, static_cast<int>(sr / m_minPitch));

    if (minLag >= maxLag) return result;

    // Voicing decision
    result.isVoiced = decideVoiced(acf, frame);

    if (!result.isVoiced) {
        result.frequency = 0.0;
        result.confidence = 0.0;
    } else {
        // Find peak
        int peakIdx = findPeakInRange(acf, minLag, maxLag);
        if (peakIdx > 0) {
            // Parabolic interpolation for sub-sample accuracy
            double refinedLag = parabolicInterpolation(acf, peakIdx);
            double freq = sr / refinedLag;
            double samplePeriod = 1.0 / sr;

            // Octave correction
            freq = octaveCorrect(freq, acf, samplePeriod);

            result.frequency = freq;
            result.period = refinedLag;
            result.clarity = (acf[0] > 1e-12) ? acf[peakIdx] / acf[0] : 0.0;
            result.confidence = qBound(0.0, result.clarity, 1.0);
        }
    }

    m_stats.totalFrames++;
    m_stats.sampleRate = m_sampleRate;
    m_stats.minPitch = m_minPitch;
    m_stats.maxPitch = m_maxPitch;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit pitchDetected(result.frequency, result.isVoiced, result.confidence);
    return result;
}

/* ---- Multi-frame detection ---- */

QVector<PitchDetector3::PitchResult> PitchDetector3::detectMulti(
    const QVector<QVector<double>>& frames) const
{
    QVector<PitchResult> results;
    results.reserve(frames.size());
    for (const auto& frame : frames)
        results.append(detect(frame));
    return results;
}

/* ---- Reset ---- */

void PitchDetector3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
