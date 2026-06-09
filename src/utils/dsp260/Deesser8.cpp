/**
 * @file Deesser8.cpp
 * @brief Deesser8 实现
 *
 * 实现去齿音器：频谱通量唇齿音检测与4-10kHz频段感知压缩。
 */

#include "utils/dsp260/Deesser8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Deesser8::Deesser8(QObject *parent)
    : QObject(parent)
{
    // Precompute Hann window
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
}
Deesser8::~Deesser8() = default;

/* ---- Configuration ---- */

void Deesser8::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); }
void Deesser8::setThreshold(double thresholdDb) { m_thresholdDb = thresholdDb; }
void Deesser8::setRatio(double ratio) { m_ratio = qMax(1.0, ratio); }
void Deesser8::setAttack(double ms) { m_attackMs = qMax(0.1, ms); }
void Deesser8::setRelease(double ms) { m_releaseMs = qMax(1.0, ms); }

/* ---- Hann window ---- */

void Deesser8::applyWindow(QVector<double>& frame) const
{
    int n = qMin(frame.size(), m_window.size());
    for (int i = 0; i < n; ++i)
        frame[i] *= m_window[i];
}

/* ---- Simple DFT ---- */

void Deesser8::dft(const QVector<double>& input, QVector<double>& real, QVector<double>& imag) const
{
    int n = qMin(input.size(), m_fftSize);
    int numBins = n / 2 + 1;
    real.resize(numBins);
    imag.resize(numBins);
    for (int k = 0; k < numBins; ++k) {
        double rSum = 0.0, iSum = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            rSum += input[i] * qCos(angle);
            iSum += input[i] * qSin(angle);
        }
        real[k] = rSum;
        imag[k] = iSum;
    }
}

/* ---- Magnitude spectrum ---- */

QVector<double> Deesser8::magnitudeSpectrum(const QVector<double>& frame) const
{
    QVector<double> windowed = frame;
    windowed.resize(m_fftSize, 0.0);
    applyWindow(windowed);

    QVector<double> real, imag;
    dft(windowed, real, imag);

    QVector<double> mag(real.size());
    for (int i = 0; i < real.size(); ++i)
        mag[i] = qSqrt(real[i] * real[i] + imag[i] * imag[i]);
    return mag;
}

/* ---- Spectral flux in sibilance band ---- */

double Deesser8::spectralFlux(const QVector<double>& mag) const
{
    double binHz = m_sampleRate / m_fftSize;
    int loBin = qMax(1, static_cast<int>(4000.0 / binHz));
    int hiBin = qMin(mag.size() - 1, static_cast<int>(10000.0 / binHz));

    double flux = 0.0;
    for (int k = loBin; k <= hiBin; ++k) {
        double diff = mag[k];
        if (m_prevMagnitude.size() > k)
            diff -= m_prevMagnitude[k];
        // Only positive changes (onset detection)
        if (diff > 0.0)
            flux += diff * diff;
    }
    return qSqrt(flux / qMax(hiBin - loBin + 1, 1));
}

/* ---- Compute gain from envelope ---- */

double Deesser8::computeGain(double inputDb) const
{
    if (inputDb <= m_thresholdDb) return 0.0;
    double overDb = inputDb - m_thresholdDb;
    return -overDb * (1.0 - 1.0 / m_ratio);
}

/* ---- Detect sibilance ---- */

double Deesser8::detectSibilance(const QVector<double>& frame) const
{
    QVector<double> mag = magnitudeSpectrum(frame);
    double flux = spectralFlux(mag);

    // Also check absolute energy in 4-10kHz band
    double binHz = m_sampleRate / m_fftSize;
    int loBin = qMax(1, static_cast<int>(4000.0 / binHz));
    int hiBin = qMin(mag.size() - 1, static_cast<int>(10000.0 / binHz));
    double energy = 0.0;
    for (int k = loBin; k <= hiBin; ++k)
        energy += mag[k] * mag[k];

    // Combined sibilance score
    double rmsEnergy = qSqrt(energy / qMax(hiBin - loBin + 1, 1));
    return flux * qLn(qMax(rmsEnergy, 1e-10) + 1.0);
}

/* ---- Process frame ---- */

QVector<double> Deesser8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output = input;
    if (input.isEmpty()) return output;

    // Compute magnitude spectrum
    QVector<double> mag = magnitudeSpectrum(input);

    // Compute sibilance level
    double sibLevel = spectralFlux(mag);

    // Convert to dB
    double rmsDb = 20.0 * qLn(qMax(sibLevel, 1e-10)) / M_LN10;

    // Compute target gain reduction
    double targetGain = computeGain(rmsDb);

    // Smooth envelope with attack/release
    double coeff = (targetGain < m_gainReductionDb) ? m_attackMs : m_releaseMs;
    double alpha = qExp(-1.0 / (m_sampleRate / input.size() * coeff * 0.001));
    m_gainReductionDb = alpha * m_gainReductionDb + (1.0 - alpha) * targetGain;

    // Apply gain reduction to sibilance band
    double gainLin = qPow(10.0, m_gainReductionDb / 20.0);
    double binHz = m_sampleRate / m_fftSize;
    int loBin = qMax(0, static_cast<int>(4000.0 / binHz));
    int hiBin = qMin(mag.size() - 1, static_cast<int>(10000.0 / binHz));

    // Reconstruct via spectral subtraction in sibilance band
    // Simplified: apply gain to the affected frequency range via time-domain scaling
    double totalEnergy = 0.0;
    double sibEnergy = 0.0;
    for (int i = 0; i < mag.size(); ++i) {
        totalEnergy += mag[i] * mag[i];
        if (i >= loBin && i <= hiBin)
            sibEnergy += mag[i] * mag[i];
    }
    double sibRatio = (totalEnergy > 0.0) ? sibEnergy / totalEnergy : 0.0;
    double frameGain = 1.0 - sibRatio * (1.0 - gainLin);
    for (int i = 0; i < output.size(); ++i)
        output[i] *= frameGain;

    // Store magnitude for next frame spectral flux
    m_prevMagnitude = mag;
    m_frameCount++;

    double elapsed = timer.elapsed();
    bool detected = (rmsDb > m_thresholdDb);
    m_stats.numFramesProcessed++;
    if (detected) {
        m_stats.numSibilanceDetected++;
        emit sibilanceDetected(m_frameCount, rmsDb, m_gainReductionDb);
    }
    m_reductionSum += qAbs(m_gainReductionDb);
    m_stats.avgReductionDb = m_reductionSum / m_stats.numFramesProcessed;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit frameProcessed(m_frameCount, elapsed);
    return output;
}

/* ---- Gain reduction accessor ---- */

double Deesser8::gainReduction() const { return m_gainReductionDb; }

/* ---- Reset ---- */

void Deesser8::resetStatistics()
{
    m_prevMagnitude.clear();
    m_gainReductionDb = 0.0;
    m_envelope = 0.0;
    m_frameCount = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reductionSum = 0.0;
}
