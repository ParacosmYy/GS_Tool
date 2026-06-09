/**
 * @file SpectralGate5.cpp
 * @brief SpectralGate5 实现
 *
 * 实现频谱门限降噪：幅度阈值二值掩码与时频平滑抑制音乐噪声。
 */

#include "utils/dsp237/SpectralGate5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralGate5::SpectralGate5(QObject *parent) : QObject(parent) {}
SpectralGate5::~SpectralGate5() = default;

/* ---- Configuration ---- */

void SpectralGate5::setFrameSize(int size) { m_frameSize = qMax(64, size); }
void SpectralGate5::setHopSize(int hop) { m_hopSize = qMax(1, hop); }
void SpectralGate5::setThreshold(double thresholdDb) { m_thresholdDb = thresholdDb; }
void SpectralGate5::setSmoothing(double factor) { m_smoothing = qBound(0.0, factor, 1.0); }
void SpectralGate5::setNoiseEstimationTime(double seconds) { m_noiseTime = qMax(0.1, seconds); }
void SpectralGate5::setMode(GateMode mode) { m_mode = mode; }

/* ---- Compute Hann window ---- */

void SpectralGate5::computeWindow()
{
    m_window.resize(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_frameSize - 1)));
}

/* ---- Radix-2 FFT ---- */

void SpectralGate5::fft(QVector<double>& re, QVector<double>& im, bool inverse)
{
    int n = re.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]); std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len * (inverse ? 1.0 : -1.0);
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uRe = re[i + j], uIm = im[i + j];
                double vRe = re[i + j + len / 2] * curRe - im[i + j + len / 2] * curIm;
                double vIm = re[i + j + len / 2] * curIm + im[i + j + len / 2] * curRe;
                re[i + j] = uRe + vRe; im[i + j] = uIm + vIm;
                re[i + j + len / 2] = uRe - vRe; im[i + j + len / 2] = uIm - vIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
    if (inverse) for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
}

/* ---- Polar conversion ---- */

void SpectralGate5::toPolar(const QVector<double>& re, const QVector<double>& im,
                             QVector<double>& mag, QVector<double>& phase) const
{
    int n = re.size();
    mag.resize(n); phase.resize(n);
    for (int i = 0; i < n; ++i) {
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
        phase[i] = qAtan2(im[i], re[i]);
    }
}

void SpectralGate5::toComplex(const QVector<double>& mag, const QVector<double>& phase,
                               QVector<double>& re, QVector<double>& im) const
{
    int n = mag.size();
    re.resize(n); im.resize(n);
    for (int i = 0; i < n; ++i) {
        re[i] = mag[i] * qCos(phase[i]);
        im[i] = mag[i] * qSin(phase[i]);
    }
}

/* ---- Apply gate ---- */

QVector<double> SpectralGate5::applyGate(const QVector<double>& magnitude) const
{
    int n = magnitude.size();
    QVector<double> gated(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double threshold = m_noiseSpectrum.isEmpty()
            ? qPow(10.0, m_thresholdDb / 20.0)
            : m_noiseSpectrum[i % m_noiseSpectrum.size()];
        switch (m_mode) {
        case BinaryMask:
            gated[i] = (magnitude[i] >= threshold) ? magnitude[i] : 0.0;
            break;
        case SoftSpectralSubtract: {
            double diff = magnitude[i] * magnitude[i] - threshold * threshold;
            gated[i] = (diff > 0) ? qSqrt(diff) : 0.0;
            break;
        }
        case WienerFilter: {
            double snr = (threshold > 1e-10) ? magnitude[i] / threshold - 1.0 : 0.0;
            double gain = qMax(0.0, snr / (1.0 + qMax(0.0, snr)));
            gated[i] = magnitude[i] * gain;
            break;
        }
        }
    }
    return gated;
}

/* ---- Smooth mask ---- */

QVector<double> SpectralGate5::smoothMask(const QVector<double>& mask) const
{
    int n = mask.size();
    QVector<double> smoothed(n, 0.0);
    // Temporal smoothing with previous frame
    for (int i = 0; i < n; ++i) {
        double prev = (i < m_prevMask.size()) ? m_prevMask[i] : mask[i];
        smoothed[i] = m_smoothing * prev + (1.0 - m_smoothing) * mask[i];
    }
    // Frequency-domain smoothing (3-bin moving average)
    for (int i = 1; i < n - 1; ++i)
        smoothed[i] = (smoothed[i - 1] + smoothed[i] + smoothed[i + 1]) / 3.0;
    return smoothed;
}

/* ---- Overlap-add ---- */

void SpectralGate5::overlapAdd(const QVector<double>& frame, int pos)
{
    for (int i = 0; i < frame.size(); ++i) {
        int idx = pos + i;
        if (idx >= 0 && idx < m_outputBuffer.size())
            m_outputBuffer[idx] += frame[i] * m_window[i];
    }
}

/* ---- Compute SNR ---- */

double SpectralGate5::computeSNR(const QVector<double>& clean, const QVector<double>& noisy) const
{
    double signalPow = 0.0, noisePow = 0.0;
    for (int i = 0; i < clean.size(); ++i) {
        signalPow += clean[i] * clean[i];
        double n = noisy[i] - clean[i];
        noisePow += n * n;
    }
    return (noisePow > 1e-15) ? 10.0 * qLog10(signalPow / noisePow) : 100.0;
}

/* ---- Estimate noise ---- */

void SpectralGate5::estimateNoise(const QVector<double>& noiseSegment)
{
    int n = noiseSegment.size();
    int bins = m_frameSize / 2 + 1;
    m_noiseSpectrum.resize(bins, 0.0);
    int numFrames = n / m_hopSize;

    for (int f = 0; f < numFrames; ++f) {
        QVector<double> re(m_frameSize, 0.0), im(m_frameSize, 0.0);
        for (int i = 0; i < m_frameSize && f * m_hopSize + i < n; ++i)
            re[i] = noiseSegment[f * m_hopSize + i] * m_window[i];
        fft(re, im, false);
        for (int k = 0; k < bins; ++k)
            m_noiseSpectrum[k] += qSqrt(re[k] * re[k] + im[k] * im[k]);
    }
    for (int k = 0; k < bins; ++k)
        m_noiseSpectrum[k] = (numFrames > 0) ? m_noiseSpectrum[k] / numFrames : 0.0;
}

/* ---- Process ---- */

QVector<double> SpectralGate5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    computeWindow();
    int n = input.size();
    m_outputBuffer.resize(n, 0.0);
    int bins = m_frameSize / 2 + 1;
    m_prevMask.clear();

    int numFrames = 0;
    for (int pos = 0; pos + m_frameSize <= n; pos += m_hopSize) {
        // Window and FFT
        QVector<double> re(m_frameSize, 0.0), im(m_frameSize, 0.0);
        for (int i = 0; i < m_frameSize; ++i)
            re[i] = input[pos + i] * m_window[i];
        fft(re, im, false);

        QVector<double> mag, phase;
        toPolar(re, im, mag, phase);

        // Truncate to positive frequencies
        mag = mag.mid(0, bins);
        phase = phase.mid(0, bins);

        // Apply gate and smooth
        QVector<double> gated = applyGate(mag);
        QVector<double> smoothed = smoothMask(gated);
        m_prevMask = smoothed;

        // Reconstruct spectrum
        QVector<double> newRe(m_frameSize, 0.0), newIm(m_frameSize, 0.0);
        for (int k = 0; k < bins; ++k) {
            newRe[k] = smoothed[k] * qCos(phase[k]);
            newIm[k] = smoothed[k] * qSin(phase[k]);
        }
        for (int k = 1; k < bins - 1; ++k) {
            newRe[m_frameSize - k] = newRe[k];
            newIm[m_frameSize - k] = -newIm[k];
        }

        // IFFT
        fft(newRe, newIm, true);

        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i)
            frame[i] = newRe[i] * m_window[i];
        overlapAdd(frame, pos);

        numFrames++;
        double snr = computeSNR(gated, mag);
        emit frameProcessed(numFrames, snr);
    }

    m_stats.frameSize = m_frameSize;
    m_stats.hopSize = m_hopSize;
    m_stats.numFrames = numFrames;
    m_stats.noiseFloor = m_noiseSpectrum.isEmpty() ? 0.0 : *std::min_element(m_noiseSpectrum.begin(), m_noiseSpectrum.end());
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(numFrames, timer.elapsed());
    return m_outputBuffer;
}

/* ---- Accessors ---- */

QVector<double> SpectralGate5::noiseSpectrum() const { return m_noiseSpectrum; }

/* ---- Reset ---- */

void SpectralGate5::resetStatistics()
{
    m_noiseSpectrum.clear(); m_window.clear(); m_prevMask.clear();
    m_outputBuffer.clear(); m_overlapBuf.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
