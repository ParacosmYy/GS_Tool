/**
 * @file PitchDetector8.cpp
 * @brief PitchDetector8 实现
 *
 * 实现基频检测器：倒谱法与能量零交叉率清浊音判决。
 */

#include "utils/signal260/PitchDetector8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PitchDetector8::PitchDetector8(QObject *parent)
    : QObject(parent) {}
PitchDetector8::~PitchDetector8() = default;

/* ---- Configuration ---- */

void PitchDetector8::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); }
void PitchDetector8::setPitchRange(double minHz, double maxHz)
{
    m_minPitch = qMax(20.0, minHz);
    m_maxPitch = qMin(m_sampleRate / 2.0, maxHz);
}
void PitchDetector8::setFrameSize(int samples) { m_frameSize = qMax(64, samples); }
void PitchDetector8::setHopSize(int samples) { m_hopSize = qMax(1, samples); }
void PitchDetector8::setEnergyThreshold(double threshold) { m_energyThreshold = threshold; }
void PitchDetector8::setZCRThreshold(double threshold) { m_zcrThreshold = threshold; }

/* ---- Apply Hanning window ---- */

QVector<double> PitchDetector8::applyWindow(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> windowed(n);
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        windowed[i] = frame[i] * w;
    }
    return windowed;
}

/* ---- Compute energy (RMS) ---- */

double PitchDetector8::computeEnergy(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double s : frame) sum += s * s;
    return qSqrt(sum / frame.size());
}

/* ---- Compute zero-crossing rate ---- */

double PitchDetector8::computeZCR(const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;
    int crossings = 0;
    for (int i = 1; i < frame.size(); ++i) {
        if ((frame[i] >= 0.0) != (frame[i - 1] >= 0.0))
            crossings++;
    }
    return static_cast<double>(crossings) / (frame.size() - 1);
}

/* ---- Log power spectrum ---- */

QVector<double> PitchDetector8::logPowerSpectrum(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> windowed = applyWindow(frame);

    // DFT for power spectrum
    int halfN = n / 2;
    QVector<double> powerSpec(halfN, 0.0);
    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            re += windowed[i] * qCos(angle);
            im += windowed[i] * qSin(angle);
        }
        powerSpec[k] = re * re + im * im;
    }

    // Log spectrum
    QVector<double> logSpec(halfN);
    for (int k = 0; k < halfN; ++k)
        logSpec[k] = qLn(qMax(powerSpec[k], 1e-20));

    return logSpec;
}

/* ---- Real cepstrum ---- */

QVector<double> PitchDetector8::cepstrum(const QVector<double>& frame) const
{
    QVector<double> logSpec = logPowerSpectrum(frame);
    int n = logSpec.size();

    // Inverse DFT of log spectrum to get cepstrum
    QVector<double> ceps(n, 0.0);
    for (int q = 0; q < n; ++q) {
        double sum = 0.0;
        for (int k = 0; k < n; ++k) {
            sum += logSpec[k] * qCos(2.0 * M_PI * k * q / n);
        }
        ceps[q] = sum / n;
    }
    return ceps;
}

/* ---- Find pitch peak in cepstrum ---- */

double PitchDetector8::findPitchPeak(const QVector<double>& ceps) const
{
    // Quefrency range corresponding to pitch range
    int minQ = qMax(1, static_cast<int>(m_sampleRate / m_maxPitch));
    int maxQ = qMin(ceps.size() - 1, static_cast<int>(m_sampleRate / m_minPitch));

    if (minQ >= maxQ || minQ < 1 || maxQ >= ceps.size()) return 0.0;

    double peakVal = -std::numeric_limits<double>::max();
    int peakIdx = minQ;
    for (int q = minQ; q <= maxQ; ++q) {
        if (ceps[q] > peakVal) {
            peakVal = ceps[q];
            peakIdx = q;
        }
    }

    // Parabolic interpolation for sub-sample accuracy
    if (peakIdx > 0 && peakIdx < ceps.size() - 1) {
        double alpha = ceps[peakIdx - 1];
        double beta = ceps[peakIdx];
        double gamma = ceps[peakIdx + 1];
        double denom = alpha - 2.0 * beta + gamma;
        if (qFabs(denom) > 1e-14) {
            double delta = 0.5 * (alpha - gamma) / denom;
            peakIdx = static_cast<int>(peakIdx + delta);
            if (peakIdx < minQ) peakIdx = minQ;
            if (peakIdx > maxQ) peakIdx = maxQ;
        }
    }

    double pitch = (peakIdx > 0) ? m_sampleRate / peakIdx : 0.0;
    return (pitch >= m_minPitch && pitch <= m_maxPitch) ? pitch : 0.0;
}

/* ---- Analyze single frame ---- */

PitchDetector8::FrameResult PitchDetector8::analyzeFrame(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    FrameResult result;

    // Compute energy and ZCR for voiced/unvoiced decision
    result.energy = computeEnergy(frame);
    result.zeroCrossingRate = computeZCR(frame);

    // Voiced/unvoiced classification
    result.isVoiced = (result.energy >= m_energyThreshold)
                      && (result.zeroCrossingRate <= m_zcrThreshold);

    // Pitch detection via cepstrum
    if (result.isVoiced) {
        QVector<double> ceps = cepstrum(frame);
        result.pitchHz = findPitchPeak(ceps);

        // Confidence based on cepstral peak prominence
        if (result.pitchHz > 0.0) {
            int peakQ = qMax(1, static_cast<int>(m_sampleRate / result.pitchHz));
            if (peakQ < ceps.size()) {
                double peakVal = ceps[peakQ];
                double meanCeps = 0.0;
                int count = 0;
                for (int i = 1; i < ceps.size(); ++i) {
                    meanCeps += ceps[i];
                    count++;
                }
                meanCeps = (count > 0) ? meanCeps / count : 0.0;
                result.confidence = qBound(0.0, (peakVal - meanCeps)
                    / (qFabs(peakVal) + qFabs(meanCeps) + 1e-10), 1.0);
            }
        }
    } else {
        result.pitchHz = 0.0;
        result.confidence = 0.0;
    }

    double elapsed = timer.elapsed();
    m_stats.framesAnalyzed++;
    if (result.isVoiced) m_stats.voicedFrames++;
    else m_stats.unvoicedFrames++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit frameAnalyzed(result.pitchHz, result.isVoiced, elapsed);
    return result;
}

/* ---- Analyze entire signal ---- */

QVector<PitchDetector8::FrameResult> PitchDetector8::analyze(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<FrameResult> results;
    int numFrames = (signal.size() - m_frameSize) / m_hopSize + 1;
    if (numFrames <= 0) return results;

    results.reserve(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize && (start + i) < signal.size(); ++i)
            frame[i] = signal[start + i];
        results.append(analyzeFrame(frame));
    }

    double elapsed = timer.elapsed();
    emit analysisCompleted(results.size(), m_stats.voicedFrames, elapsed);
    return results;
}

/* ---- Reset ---- */

void PitchDetector8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
