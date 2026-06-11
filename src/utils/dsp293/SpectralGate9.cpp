/**
 * @file SpectralGate9.cpp
 * @brief SpectralGate9 实现
 *
 * 实现频谱门限：噪声底估计与时频掩蔽实现语音增强中的音乐噪声抑制。
 */

#include "utils/dsp293/SpectralGate9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralGate9::SpectralGate9(QObject *parent)
    : QObject(parent)
{
    generateWindow();
}

SpectralGate9::~SpectralGate9() = default;

/* ---- Configuration ---- */

void SpectralGate9::setFftSize(int size)
{
    m_fftSize = qBound(64, size, 8192);
    generateWindow();
    m_prevPhase.resize(m_fftSize / 2 + 1, 0.0);
    m_prevTail.resize(m_fftSize / 2, 0.0);
}

void SpectralGate9::setOverwriteFactor(double factor) { m_overlapFactor = qBound(0.0, factor, 0.75); }
void SpectralGate9::setNoiseThreshold(double thresh) { m_noiseThreshold = qBound(0.0, thresh, 30.0); }
void SpectralGate9::setSpectralFloor(double floor) { m_spectralFloor = qBound(0.001, floor, 0.5); }
void SpectralGate9::setNoiseEstimationFrames(int frames) { m_noiseFrames = qBound(1, frames, 100); }

/* ---- Generate Hann window ---- */

void SpectralGate9::generateWindow()
{
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
}

/* ---- Simple DFT ---- */

void SpectralGate9::dft(const QVector<double>& input,
                          QVector<double>& real, QVector<double>& imag) const
{
    int N = input.size();
    int halfN = N / 2 + 1;
    real.resize(halfN);
    imag.resize(halfN);
    for (int k = 0; k < halfN; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            sr += input[n] * qCos(angle);
            si += input[n] * qSin(angle);
        }
        real[k] = sr;
        imag[k] = si;
    }
}

/* ---- Inverse DFT (full synthesis from half-spectrum) ---- */

void SpectralGate9::idft(const QVector<double>& real, const QVector<double>& imag,
                            QVector<double>& output) const
{
    int halfN = real.size();
    int N = (halfN - 1) * 2;
    output.resize(N);
    for (int n = 0; n < N; ++n) {
        double s = real[0]; // DC component (imag=0)
        for (int k = 1; k < halfN - 1; ++k) {
            double angle = 2.0 * M_PI * k * n / N;
            s += 2.0 * (real[k] * qCos(angle) - imag[k] * qSin(angle));
        }
        // Nyquist bin
        double angle = 2.0 * M_PI * (halfN - 1) * n / N;
        s += real[halfN - 1] * qCos(angle);
        output[n] = s / N;
    }
}

/* ---- Compute spectrum ---- */

void SpectralGate9::computeSpectrum(const QVector<double>& frame,
                                      QVector<double>& magnitude,
                                      QVector<double>& phase) const
{
    // Apply window
    QVector<double> windowed(m_fftSize);
    for (int i = 0; i < qMin(frame.size(), m_fftSize); ++i)
        windowed[i] = frame[i] * m_window[i];

    QVector<double> real, imag;
    dft(windowed, real, imag);

    int bins = real.size();
    magnitude.resize(bins);
    phase.resize(bins);
    for (int k = 0; k < bins; ++k) {
        magnitude[k] = qSqrt(real[k] * real[k] + imag[k] * imag[k]);
        phase[k] = qAtan2(imag[k], real[k]);
    }
}

/* ---- Reconstruct signal ---- */

QVector<double> SpectralGate9::reconstructSignal(const QVector<double>& magnitude,
                                                   const QVector<double>& phase) const
{
    int bins = magnitude.size();
    QVector<double> real(bins), imag(bins);
    for (int k = 0; k < bins; ++k) {
        real[k] = magnitude[k] * qCos(phase[k]);
        imag[k] = magnitude[k] * qSin(phase[k]);
    }
    QVector<double> output;
    idft(real, imag, output);
    // Apply synthesis window
    for (int i = 0; i < output.size(); ++i)
        output[i] *= m_window[i];
    return output;
}

/* ---- Build spectral mask ---- */

QVector<double> SpectralGate9::buildMask(const QVector<double>& magnitude) const
{
    int bins = magnitude.size();
    QVector<double> mask(bins, m_spectralFloor);

    if (!m_noiseEstimated) return mask;

    for (int k = 0; k < bins; ++k) {
        double noisePow = qMax(m_noiseSpectrum[k], 1e-15);
        double magPow = magnitude[k] * magnitude[k];
        double snrDb = 10.0 * qLn(magPow / noisePow) / M_LN10;

        // Soft gating: smooth transition around threshold
        if (snrDb > m_noiseThreshold + 6.0) {
            mask[k] = 1.0;
        } else if (snrDb > m_noiseThreshold - 6.0) {
            // Sigmoid-like transition to reduce musical noise
            double t = (snrDb - m_noiseThreshold + 6.0) / 12.0;
            mask[k] = m_spectralFloor + (1.0 - m_spectralFloor) * t;
        }
        // Below threshold: keep spectral floor
    }
    return mask;
}

/* ---- Estimate noise from silence frames ---- */

void SpectralGate9::estimateNoise(const QVector<QVector<double>>& silenceFrames)
{
    int n = silenceFrames.size();
    if (n == 0) return;

    int bins = m_fftSize / 2 + 1;
    m_noiseSpectrum.resize(bins);
    m_noiseSpectrum.fill(0.0);

    for (int f = 0; f < n; ++f) {
        QVector<double> mag, ph;
        computeSpectrum(silenceFrames[f], mag, ph);
        for (int k = 0; k < bins; ++k)
            m_noiseSpectrum[k] += mag[k] * mag[k];
    }
    for (int k = 0; k < bins; ++k)
        m_noiseSpectrum[k] /= n;

    m_noiseEstimated = true;
}

/* ---- Process a frame ---- */

SpectralGate9::GateResult SpectralGate9::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    GateResult result;
    int n = qMin(frame.size(), m_fftSize);
    if (n == 0) return result;

    // Analyze spectrum
    QVector<double> padded(m_fftSize, 0.0);
    for (int i = 0; i < n; ++i) padded[i] = frame[i];

    QVector<double> magnitude, phase;
    computeSpectrum(padded, magnitude, phase);

    // Build mask and apply
    result.mask = buildMask(magnitude);
    QVector<double> gatedMag(magnitude.size());
    for (int k = 0; k < magnitude.size(); ++k)
        gatedMag[k] = magnitude[k] * result.mask[k];

    // Store noise floor info
    result.noiseFloor = m_noiseSpectrum;

    // Compute SNR improvement estimate
    double signalEnergy = 0.0, gatedEnergy = 0.0, noiseEnergy = 0.0;
    for (int k = 0; k < magnitude.size(); ++k) {
        signalEnergy += magnitude[k] * magnitude[k];
        gatedEnergy += gatedMag[k] * gatedMag[k];
        noiseEnergy += m_noiseEstimated ? m_noiseSpectrum[k] : 0.0;
    }
    if (noiseEnergy > 1e-15 && signalEnergy > 1e-15)
        result.snrImprovementDb = 10.0 * qLn(gatedEnergy / qMax(noiseEnergy, 1e-15)) / M_LN10
                                 - 10.0 * qLn(signalEnergy / qMax(noiseEnergy, 1e-15)) / M_LN10;
    else
        result.snrImprovementDb = 0.0;

    // Reconstruct with overlap-add
    auto synth = reconstructSignal(gatedMag, phase);
    int hopSize = m_fftSize / 2;

    result.output.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        result.output[i] = synth[i];

    // Add overlap from previous frame
    for (int i = 0; i < qMin(m_prevTail.size(), result.output.size()); ++i)
        result.output[i] += m_prevTail[i];

    // Save tail for next frame
    m_prevTail.resize(hopSize);
    for (int i = 0; i < hopSize; ++i)
        m_prevTail[i] = result.output[i + hopSize];

    double elapsed = timer.elapsed();
    m_stats.fftSize = m_fftSize;
    m_stats.totalFrames++;
    m_snrSum += result.snrImprovementDb;
    m_stats.avgSnrImprovementDb = m_snrSum / m_stats.totalFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(n, result.snrImprovementDb, elapsed);
    return result;
}

/* ---- Reset noise ---- */

void SpectralGate9::resetNoise()
{
    m_noiseSpectrum.clear();
    m_noiseEstimated = false;
}

/* ---- Reset statistics ---- */

void SpectralGate9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_snrSum = 0.0;
    resetNoise();
}
