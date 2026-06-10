/**
 * @file NoiseGate8.cpp
 * @brief NoiseGate8 实现
 *
 * 实现噪声门：谱减法预处理与自适应阈值噪声底跟踪。
 */

#include "utils/dsp261/NoiseGate8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

NoiseGate8::NoiseGate8(QObject *parent)
    : QObject(parent) { initWindow(); }
NoiseGate8::~NoiseGate8() = default;

/* ---- Configuration ---- */

void NoiseGate8::setFrameSize(int size) { m_frameSize = qMax(64, size); initWindow(); }
void NoiseGate8::setBaseThresholdDb(double thresholdDb) { m_baseThresholdDb = thresholdDb; }
void NoiseGate8::setNoiseAlpha(double alpha) { m_noiseAlpha = qBound(0.0, alpha, 1.0); }
void NoiseGate8::setSpectralSubtractionFactor(double factor) { m_subtractFactor = qMax(0.0, factor); }

/* ---- Initialize Hann window ---- */

void NoiseGate8::initWindow()
{
    m_window.resize(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_frameSize));
}

/* ---- Apply Hann window ---- */

QVector<double> NoiseGate8::applyWindow(const QVector<double>& frame) const
{
    int n = qMin(frame.size(), m_window.size());
    QVector<double> out(n);
    for (int i = 0; i < n; ++i)
        out[i] = frame[i] * m_window[i];
    return out;
}

/* ---- Compute magnitude spectrum via DFT ---- */

QVector<double> NoiseGate8::magnitudeSpectrum(const QVector<double>& windowed) const
{
    int n = windowed.size();
    int half = n / 2 + 1;
    QVector<double> mag(half, 0.0);
    for (int k = 0; k < half; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            re += windowed[i] * qCos(angle);
            im += windowed[i] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
    return mag;
}

/* ---- Spectral subtraction ---- */

QVector<double> NoiseGate8::spectralSubtract(const QVector<double>& magnitude) const
{
    int n = magnitude.size();
    QVector<double> cleaned(n);
    for (int i = 0; i < n; ++i) {
        double estimate = magnitude[i] - m_subtractFactor * m_noiseSpectrum.value(i, 0.0);
        cleaned[i] = qMax(estimate, m_spectralFloor * magnitude[i]);
    }
    return cleaned;
}

/* ---- Update noise floor estimate ---- */

void NoiseGate8::updateNoiseFloor(const QVector<double>& magnitude)
{
    if (m_noiseSpectrum.size() != magnitude.size()) {
        m_noiseSpectrum = magnitude;
        return;
    }
    // Exponential moving average for noise tracking
    for (int i = 0; i < magnitude.size(); ++i) {
        m_noiseSpectrum[i] = m_noiseAlpha * m_noiseSpectrum[i]
                             + (1.0 - m_noiseAlpha) * magnitude[i];
    }
}

/* ---- Compute adaptive threshold ---- */

double NoiseGate8::computeAdaptiveThreshold() const
{
    // Adaptive threshold: base + noise floor offset
    return m_baseThresholdDb + m_noiseFloor;
}

/* ---- RMS energy in dB ---- */

double NoiseGate8::rmsDb(const QVector<double>& samples) const
{
    double sumSq = 0.0;
    for (double s : samples) sumSq += s * s;
    double rms = qSqrt(sumSq / qMax(samples.size(), 1));
    return 20.0 * qLog10(qMax(rms, 1e-10));
}

/* ---- Reconstruct time-domain from modified spectrum ---- */

QVector<double> NoiseGate8::reconstruct(const QVector<double>& modifiedMag,
                                          const QVector<double>& originalFrame) const
{
    int n = originalFrame.size();
    int half = modifiedMag.size();

    // Compute original phase
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double re = 0.0, im = 0.0;
        // Use magnitude-weighted overlap-add with original phase
        for (int k = 0; k < half; ++k) {
            double angle = 2.0 * M_PI * k * i / n;
            // Get original phase from DFT of original
            double origRe = 0.0, origIm = 0.0;
            for (int j = 0; j < n; ++j) {
                double a = -2.0 * M_PI * k * j / n;
                origRe += originalFrame[j] * qCos(a);
                origIm += originalFrame[j] * qSin(a);
            }
            double origMag = qSqrt(origRe * origRe + origIm * origIm);
            double scale = (origMag > 1e-10) ? modifiedMag[k] / origMag : 0.0;
            re += scale * origRe * qCos(angle) - scale * origIm * qSin(angle);
        }
        result[i] = re / n;
    }
    return result;
}

/* ---- Learn noise profile ---- */

void NoiseGate8::learnNoiseProfile(const QVector<double>& noiseSegment)
{
    QVector<double> windowed = applyWindow(noiseSegment);
    m_noiseSpectrum = magnitudeSpectrum(windowed);

    // Compute overall noise floor
    double sum = 0.0;
    for (double m : m_noiseSpectrum) sum += m;
    m_noiseFloor = (sum > 0) ? 20.0 * qLog10(qMax(sum / m_noiseSpectrum.size(), 1e-10)) : -60.0;

    emit noiseProfileUpdated(m_noiseFloor);
}

/* ---- Process a frame ---- */

QVector<double> NoiseGate8::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Compute frame energy
    double frameEnergyDb = rmsDb(frame);

    // Step 2: Compute magnitude spectrum
    QVector<double> windowed = applyWindow(frame);
    QVector<double> mag = magnitudeSpectrum(windowed);

    // Step 3: Update noise floor tracking
    updateNoiseFloor(mag);

    // Step 4: Compute adaptive threshold
    double threshold = computeAdaptiveThreshold();

    QVector<double> output;
    if (frameEnergyDb < threshold) {
        // Below threshold: apply spectral subtraction for gentle gating
        QVector<double> cleaned = spectralSubtract(mag);
        output = reconstruct(cleaned, frame);
        // Apply additional attenuation
        for (int i = 0; i < output.size(); ++i)
            output[i] *= 0.1;  // -20dB attenuation when gated
    } else {
        // Above threshold: spectral subtraction for noise reduction only
        QVector<double> cleaned = spectralSubtract(mag);
        output = reconstruct(cleaned, frame);
    }

    double elapsed = timer.elapsed();
    m_stats.numFramesProcessed++;
    m_stats.avgThreshold = (m_stats.avgThreshold * (m_stats.numFramesProcessed - 1) + threshold)
                          / m_stats.numFramesProcessed;
    m_stats.noiseFloorDb = m_noiseFloor;
    m_stats.frameSize = m_frameSize;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    double snr = frameEnergyDb - m_noiseFloor;
    emit frameProcessed(m_stats.numFramesProcessed, threshold, snr, elapsed);
    return output;
}

/* ---- Accessors ---- */

double NoiseGate8::currentThresholdDb() const { return computeAdaptiveThreshold(); }

QVector<double> NoiseGate8::noiseFloorSpectrum() const { return m_noiseSpectrum; }

/* ---- Reset ---- */

void NoiseGate8::resetStatistics()
{
    m_noiseSpectrum.clear();
    m_noiseFloor = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
