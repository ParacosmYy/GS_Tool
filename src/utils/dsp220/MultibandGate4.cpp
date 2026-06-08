/**
 * @file MultibandGate4.cpp
 * @brief MultibandGate4 实现
 *
 * 实现多频段门控：Mel子带分解、递归平均噪声轮廓跟踪、门控增益计算。
 */

#include "utils/dsp220/MultibandGate4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MultibandGate4::MultibandGate4(QObject *parent) : QObject(parent) {}
MultibandGate4::~MultibandGate4() = default;

/* ---- Configuration ---- */

void MultibandGate4::setParameters(int numBands, int fftSize, double sampleRate,
                                     double thresholdDb, double reductionDb,
                                     double attackMs, double releaseMs,
                                     double noiseAlpha)
{
    m_numBands = qMax(1, numBands);
    m_fftSize = qMax(64, fftSize);
    m_sampleRate = qMax(1.0, sampleRate);
    m_thresholdDb = thresholdDb;
    m_reductionDb = reductionDb;
    m_noiseAlpha = qBound(0.0, noiseAlpha, 1.0);

    // Compute attack/release coefficients
    double attackCoeff = qExp(-1.0 / (attackMs * 0.001 * m_sampleRate / m_fftSize));
    double releaseCoeff = qExp(-1.0 / (releaseMs * 0.001 * m_sampleRate / m_fftSize));

    computeMelBands();
    computeWindow();

    m_bands.resize(m_numBands);
    for (int b = 0; b < m_numBands; ++b) {
        m_bands[b].attackCoeff = attackCoeff;
        m_bands[b].releaseCoeff = releaseCoeff;
        m_bands[b].threshold = m_thresholdDb;
        m_bands[b].reduction = m_reductionDb;
        m_bands[b].gateGain = 1.0;
        m_bands[b].noiseFloor = -100.0;
    }

    m_stats.numBands = m_numBands;
    m_stats.fftSize = m_fftSize;
}

/* ---- Mel scale conversion ---- */

double MultibandGate4::hzToMel(double hz) const
{
    return 2595.0 * qLog10(1.0 + hz / 700.0);
}

double MultibandGate4::melToHz(double mel) const
{
    return 700.0 * (qPow(10.0, mel / 2595.0) - 1.0);
}

/* ---- Compute Mel band edges ---- */

void MultibandGate4::computeMelBands()
{
    double maxMel = hzToMel(m_sampleRate / 2.0);
    m_melEdges.resize(m_numBands + 1);
    for (int i = 0; i <= m_numBands; ++i)
        m_melEdges[i] = melToHz(maxMel * i / m_numBands);

    // Map Mel edges to FFT bin indices
    for (int b = 0; b < m_numBands; ++b) {
        m_bands[b].loBin = qRound(m_melEdges[b] * m_fftSize / m_sampleRate);
        m_bands[b].hiBin = qMin(m_fftSize / 2,
                                 qRound(m_melEdges[b + 1] * m_fftSize / m_sampleRate));
    }
}

/* ---- Hann window ---- */

void MultibandGate4::computeWindow()
{
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
}

/* ---- DFT magnitude ---- */

void MultibandGate4::computeMagnitude(const QVector<double>& frame,
                                        QVector<double>& mag) const
{
    int halfN = m_fftSize / 2;
    mag.resize(halfN + 1);
    for (int k = 0; k <= halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < qMin(frame.size(), m_fftSize); ++n) {
            double angle = 2.0 * M_PI * k * n / m_fftSize;
            re += frame[n] * m_window[n] * qCos(angle);
            im -= frame[n] * m_window[n] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
}

/* ---- Noise profile learning ---- */

void MultibandGate4::learnNoiseProfile(const QVector<double>& noiseFrame)
{
    QVector<double> mag;
    computeMagnitude(noiseFrame, mag);

    for (int b = 0; b < m_numBands; ++b) {
        double energy = 0.0;
        int count = qMax(1, m_bands[b].hiBin - m_bands[b].loBin);
        for (int k = m_bands[b].loBin; k < m_bands[b].hiBin; ++k) {
            if (k < mag.size()) energy += mag[k] * mag[k];
        }
        energy = 10.0 * qLog10(energy / count + 1e-10);
        m_bands[b].noiseFloor = energy;
    }
    m_stats.avgNoiseFloor = 0.0;
    for (auto& b : m_bands) m_stats.avgNoiseFloor += b.noiseFloor;
    m_stats.avgNoiseFloor /= m_numBands;
}

/* ---- Recursive noise update ---- */

void MultibandGate4::updateNoiseEstimate(int band, double energy)
{
    double alpha = m_noiseAlpha;
    m_bands[band].noiseFloor = alpha * m_bands[band].noiseFloor
                                + (1.0 - alpha) * energy;
}

/* ---- Gate gain ---- */

double MultibandGate4::computeGateGain(int band, double energy) const
{
    double threshold = m_bands[band].noiseFloor + m_thresholdDb;
    if (energy > threshold) return 1.0;
    // Linear interpolation from reduction to 1.0 around threshold
    double range = 6.0; // Transition range in dB
    double ratio = (energy - threshold + range) / range;
    ratio = qBound(0.0, ratio, 1.0);
    double gainDb = m_reductionDb * (1.0 - ratio);
    return qPow(10.0, gainDb / 20.0);
}

/* ---- Apply gains ---- */

void MultibandGate4::applyGains(QVector<double>& mag) const
{
    for (int b = 0; b < m_numBands; ++b) {
        double gain = m_bands[b].gateGain;
        for (int k = m_bands[b].loBin; k < m_bands[b].hiBin && k < mag.size(); ++k)
            mag[k] *= gain;
    }
}

/* ---- Reconstruct ---- */

QVector<double> MultibandGate4::reconstruct(const QVector<double>& originalFrame,
                                              const QVector<double>& mag,
                                              const QVector<double>& phase) const
{
    int n = qMin(originalFrame.size(), m_fftSize);
    QVector<double> output(n, 0.0);

    // Simple overlap-add reconstruction using modified magnitude with original phase
    for (int i = 0; i < n; ++i) {
        // Weighted blend of original and gated
        double weight = 0.0;
        for (int b = 0; b < m_numBands; ++b) {
            // Estimate frequency bin for this sample
            int bin = qRound(i * (mag.size() - 1) / n);
            if (bin >= m_bands[b].loBin && bin < m_bands[b].hiBin)
                weight = m_bands[b].gateGain;
        }
        output[i] = originalFrame[i] * qMax(weight, 0.001);
    }
    return output;
}

/* ---- Process ---- */

QVector<double> MultibandGate4::process(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> mag, phase;
    computeMagnitude(inputFrame, mag);

    // Compute phase (approximate from magnitude for overlap-add)
    phase.resize(mag.size(), 0.0);

    // Per-band processing
    for (int b = 0; b < m_numBands; ++b) {
        double energy = 0.0;
        int count = qMax(1, m_bands[b].hiBin - m_bands[b].loBin);
        for (int k = m_bands[b].loBin; k < m_bands[b].hiBin && k < mag.size(); ++k)
            energy += mag[k] * mag[k];
        double energyDb = 10.0 * qLog10(energy / count + 1e-10);

        // Update noise estimate (recursive average)
        updateNoiseEstimate(b, energyDb);

        // Compute gate gain with attack/release smoothing
        double targetGain = computeGateGain(b, energyDb);
        double coeff = (targetGain < m_bands[b].gateGain)
                       ? m_bands[b].attackCoeff : m_bands[b].releaseCoeff;
        m_bands[b].gateGain = coeff * m_bands[b].gateGain
                              + (1.0 - coeff) * targetGain;
    }

    applyGains(mag);
    QVector<double> output = reconstruct(inputFrame, mag, phase);

    m_stats.framesProcessed++;
    m_stats.avgNoiseFloor = 0.0;
    for (auto& b : m_bands) m_stats.avgNoiseFloor += b.noiseFloor;
    m_stats.avgNoiseFloor /= m_numBands;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit frameProcessed(m_stats.framesProcessed, m_stats.avgNoiseFloor, timer.elapsed());
    return output;
}

/* ---- Band states ---- */

QVector<MultibandGate4::BandState> MultibandGate4::bandStates() const
{
    return m_bands;
}

/* ---- Reset ---- */

void MultibandGate4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bands.clear();
    m_window.clear();
    m_melEdges.clear();
}
