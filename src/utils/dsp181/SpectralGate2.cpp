/**
 * @file SpectralGate2.cpp
 * @brief SpectralGate2 实现
 *
 * 实现频谱门限降噪：STFT bin级门限、噪声底估计、二元掩码平滑。
 */

#include "utils/dsp181/SpectralGate2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralGate2::SpectralGate2(QObject *parent) : QObject(parent) {}
SpectralGate2::~SpectralGate2() = default;

/* ---- Configuration ---- */

void SpectralGate2::setFftSize(int size) {
    // Must be power of 2
    int s = 64;
    while (s < size) s <<= 1;
    m_fftSize = s;
}
void SpectralGate2::setHopSize(int size) { m_hopSize = qMax(1, size); }
void SpectralGate2::setThreshold(double db) { m_thresholdDb = db; }
void SpectralGate2::setNoiseEstimateFrames(int frames) { m_noiseFrames = qMax(1, frames); }
void SpectralGate2::setSmoothingWidth(int bins) { m_smoothWidth = qMax(1, bins); }
void SpectralGate2::setAttack(double ms) { m_attackMs = qMax(0.1, ms); }
void SpectralGate2::setRelease(double ms) { m_releaseMs = qMax(0.1, ms); }

/* ---- Hann window ---- */

QVector<double> SpectralGate2::hannWindow(int size) const
{
    QVector<double> w(size);
    for (int i = 0; i < size; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (size - 1)));
    return w;
}

/* ---- Magnitude spectrum ---- */

QVector<double> SpectralGate2::magnitudeSpectrum(const QVector<double>& frame) const
{
    int N = frame.size();
    int half = N / 2 + 1;
    QVector<double> mag(half, 0.0);

    // Simple DFT magnitude (half spectrum)
    for (int k = 0; k < half; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
    return mag;
}

/* ---- STFT ---- */

QVector<QVector<double>> SpectralGate2::stft(const QVector<double>& input) const
{
    QVector<QVector<double>> spectra;
    auto window = hannWindow(m_fftSize);
    int half = m_fftSize / 2 + 1;

    for (int pos = 0; pos + m_fftSize <= input.size(); pos += m_hopSize) {
        QVector<double> frame(m_fftSize);
        for (int i = 0; i < m_fftSize; ++i)
            frame[i] = input[pos + i] * window[i];

        spectra.append(magnitudeSpectrum(frame));
    }
    return spectra;
}

/* ---- ISTFT ---- */

QVector<double> SpectralGate2::istft(const QVector<QVector<double>>& spectra) const
{
    int numFrames = spectra.size();
    int outputLen = m_fftSize + (numFrames - 1) * m_hopSize;
    QVector<double> output(outputLen, 0.0);
    QVector<double> winSum(outputLen, 0.0);
    auto window = hannWindow(m_fftSize);

    for (int f = 0; f < numFrames; ++f) {
        int pos = f * m_hopSize;
        const auto& spec = spectra[f];
        int half = spec.size();

        // Simple inverse: use magnitude as real-part synthesis
        for (int i = 0; i < m_fftSize; ++i) {
            double val = 0.0;
            for (int k = 0; k < half; ++k) {
                double angle = 2.0 * M_PI * k * i / m_fftSize;
                val += spec[k] * qCos(angle);
            }
            val /= m_fftSize;
            output[pos + i] += val * window[i];
            winSum[pos + i] += window[i] * window[i];
        }
    }

    // Normalize by window sum
    for (int i = 0; i < outputLen; ++i)
        if (winSum[i] > 1e-10) output[i] /= winSum[i];

    return output;
}

/* ---- Noise floor estimation ---- */

QVector<double> SpectralGate2::estimateNoiseFloor(
    const QVector<QVector<double>>& spectra) const
{
    int half = m_fftSize / 2 + 1;
    int numFrames = qMin(m_noiseFrames, spectra.size());

    QVector<double> floor(half, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        for (int k = 0; k < half && k < spectra[f].size(); ++k)
            floor[k] += spectra[f][k];
    }
    for (int k = 0; k < half; ++k)
        floor[k] /= numFrames;

    return floor;
}

/* ---- Binary mask smoothing ---- */

QVector<double> SpectralGate2::smoothMask(const QVector<double>& mask) const
{
    int n = mask.size();
    QVector<double> smoothed(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = qMax(0, i - m_smoothWidth);
             j <= qMin(n - 1, i + m_smoothWidth); ++j) {
            sum += mask[j];
            ++count;
        }
        // Majority vote: if more than half neighbors are 1, keep 1
        smoothed[i] = (sum / count > 0.5) ? 1.0 : 0.0;
    }
    return smoothed;
}

/* ---- Main process ---- */

QVector<double> SpectralGate2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.size() < m_fftSize) return input;

    // Step 1: Compute STFT
    auto spectra = stft(input);
    int numFrames = spectra.size();
    int half = m_fftSize / 2 + 1;

    // Step 2: Estimate noise floor
    m_noiseFloor = estimateNoiseFloor(spectra);
    double avgNoiseDb = 0.0;

    // Step 3: Bin-level gating with threshold
    QVector<QVector<double>> gatedSpectra(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        QVector<double> mask(half, 0.0);
        for (int k = 0; k < half && k < spectra[f].size(); ++k) {
            double binDb = 20.0 * qLn(spectra[f][k] + 1e-10) / M_LN;
            double floorDb = 20.0 * qLn(m_noiseFloor[k] + 1e-10) / M_LN;
            double threshold = floorDb + qAbs(m_thresholdDb);

            if (k == 0) avgNoiseDb = floorDb;
            mask[k] = (binDb >= threshold) ? 1.0 : 0.0;
        }

        // Step 4: Smooth binary mask to reduce musical noise
        mask = smoothMask(mask);

        // Apply mask
        gatedSpectra[f].resize(half);
        for (int k = 0; k < half && k < spectra[f].size(); ++k)
            gatedSpectra[f][k] = spectra[f][k] * mask[k];
    }

    // Step 5: Inverse STFT
    QVector<double> output = istft(gatedSpectra);
    output.resize(input.size());

    m_stats.totalFrames += numFrames;
    m_stats.fftSize = m_fftSize;
    m_stats.hopSize = m_hopSize;
    m_stats.noiseFloorDb = avgNoiseDb;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalFrames / numFrames);

    emit processingCompleted(numFrames, avgNoiseDb);
    return output;
}

/* ---- Reset ---- */

void SpectralGate2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_noiseFloor.clear();
}
