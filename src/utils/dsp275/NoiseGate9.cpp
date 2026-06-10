/**
 * @file NoiseGate9.cpp
 * @brief NoiseGate9 实现
 *
 * 实现噪声门：双阈值迟滞与STFT频谱门控频率选择性噪声抑制。
 */

#include "utils/dsp275/NoiseGate9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

NoiseGate9::NoiseGate9(QObject *parent)
    : QObject(parent) {}

NoiseGate9::~NoiseGate9() = default;

/* ---- Configuration ---- */

void NoiseGate9::setOpenThreshold(double db) { m_openThresholdDb = db; }
void NoiseGate9::setCloseThreshold(double db) { m_closeThresholdDb = qMin(db, m_openThresholdDb); }
void NoiseGate9::setAttackTime(double ms) { m_attackMs = qBound(0.1, ms, 100.0); }
void NoiseGate9::setReleaseTime(double ms) { m_releaseMs = qBound(1.0, ms, 1000.0); }
void NoiseGate9::setFFTSize(int size)
{
    // FFT size must be power of 2, or 0 for time-domain only
    if (size == 0) { m_fftSize = 0; return; }
    int p = 1;
    while (p < size) p <<= 1;
    m_fftSize = qBound(64, p, 8192);
}
void NoiseGate9::setSpectralFloor(double db) { m_spectralFloorDb = db; }

/* ---- Compute frame RMS in dB ---- */

double NoiseGate9::frameRmsDb(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return -120.0;
    double sumSq = 0.0;
    for (double s : frame) sumSq += s * s;
    double rms = qSqrt(sumSq / frame.size());
    return (rms > 1e-15) ? 20.0 * qLn(rms) / M_LN10 : -120.0;
}

/* ---- Time-domain processing with dual-threshold hysteresis ---- */

QVector<double> NoiseGate9::processTimeDomain(const QVector<double>& input, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    int frameSize = qMax(64, static_cast<int>(sampleRate * 0.01));  // 10ms frames
    int hopSize = frameSize / 2;
    int numFrames = (n - frameSize) / hopSize + 1;
    if (numFrames < 1) numFrames = 1;

    // Compute attack/release coefficients
    double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * sampleRate / hopSize));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * sampleRate / hopSize));

    QVector<double> output(n, 0.0);
    int openFrames = 0;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        int end = qMin(start + frameSize, n);
        QVector<double> frame(end - start);
        for (int i = 0; i < frame.size(); ++i)
            frame[i] = input[start + i];

        double rmsDb = frameRmsDb(frame);

        // Dual-threshold hysteresis state machine
        switch (m_state) {
        case Closed:
            if (rmsDb > m_openThresholdDb) m_state = Opening;
            break;
        case Opening:
            m_state = Open;
            break;
        case Open:
            if (rmsDb < m_closeThresholdDb) m_state = Closing;
            break;
        case Closing:
            m_state = Closed;
            break;
        }

        // Apply gain with attack/release envelope
        double targetGain = (m_state == Open || m_state == Opening) ? 1.0 : 0.0;
        double coeff = (targetGain > m_gain) ? attackCoeff : releaseCoeff;
        m_gain = targetGain + coeff * (m_gain - targetGain);

        if (m_state == Open || m_state == Opening) openFrames++;

        // Apply gain to output
        for (int i = 0; i < frame.size() && start + i < n; ++i)
            output[start + i] += input[start + i] * m_gain;
    }

    // Normalize overlap-add
    QVector<double> window(n, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        for (int i = 0; i < frameSize && start + i < n; ++i)
            window[start + i] += 1.0;
    }
    for (int i = 0; i < n; ++i)
        if (window[i] > 0.5) output[i] /= window[i];

    double elapsed = timer.elapsed();
    m_stats.frameSize = frameSize;
    m_stats.numFrames += numFrames;
    m_stats.openRatio = (numFrames > 0) ? static_cast<double>(openFrames) / numFrames : 0.0;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(numFrames, m_stats.openRatio, elapsed);

    return output;
}

/* ---- Compute STFT ---- */

void NoiseGate9::computeSTFT(const QVector<double>& input, int hopSize,
                               QVector<QVector<double>>& magnitude,
                               QVector<QVector<double>>& phase) const
{
    int n = input.size();
    int numFrames = (n - m_fftSize) / hopSize + 1;
    if (numFrames < 1) numFrames = 1;
    int halfN = m_fftSize / 2;

    magnitude.resize(numFrames);
    phase.resize(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);

        // Apply Hann window
        for (int i = 0; i < m_fftSize && start + i < n; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
            re[i] = input[start + i] * w;
        }

        // Radix-2 FFT
        for (int i = 1, j = 0; i < m_fftSize; ++i) {
            int bit = m_fftSize >> 1;
            while (j & bit) { j ^= bit; bit >>= 1; }
            j ^= bit;
            if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
        }
        for (int len = 2; len <= m_fftSize; len <<= 1) {
            double angle = -2.0 * M_PI / len;
            for (int i = 0; i < m_fftSize; i += len) {
                for (int k = 0; k < len / 2; ++k) {
                    double wr = qCos(angle * k), wi = qSin(angle * k);
                    double tRe = re[i + k + len/2] * wr - im[i + k + len/2] * wi;
                    double tIm = re[i + k + len/2] * wi + im[i + k + len/2] * wr;
                    re[i + k + len/2] = re[i + k] - tRe;
                    im[i + k + len/2] = im[i + k] - tIm;
                    re[i + k] += tRe;
                    im[i + k] += tIm;
                }
            }
        }

        magnitude[f].resize(halfN + 1);
        phase[f].resize(halfN + 1);
        for (int b = 0; b <= halfN; ++b) {
            magnitude[f][b] = qSqrt(re[b] * re[b] + im[b] * im[b]);
            phase[f][b] = qAtan2(im[b], re[b]);
        }
    }
}

/* ---- Inverse STFT via overlap-add ---- */

QVector<double> NoiseGate9::inverseSTFT(const QVector<QVector<double>>& magnitude,
                                          const QVector<QVector<double>>& phase,
                                          int hopSize, int totalSamples) const
{
    int numFrames = magnitude.size();
    int halfN = m_fftSize / 2;
    QVector<double> output(totalSamples, 0.0);
    QVector<double> windowSum(totalSamples, 0.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        QVector<double> re(m_fftSize, 0.0), im(m_fftSize, 0.0);

        // Reconstruct complex spectrum (conjugate symmetry)
        for (int b = 0; b <= halfN; ++b) {
            re[b] = magnitude[f][b] * qCos(phase[f][b]);
            im[b] = magnitude[f][b] * qSin(phase[f][b]);
        }
        for (int b = 1; b < halfN; ++b) {
            re[m_fftSize - b] = re[b];
            im[m_fftSize - b] = -im[b];
        }

        // Inverse FFT (conjugate + forward + scale)
        for (int i = 0; i < m_fftSize; ++i) im[i] = -im[i];
        for (int i = 1, j = 0; i < m_fftSize; ++i) {
            int bit = m_fftSize >> 1;
            while (j & bit) { j ^= bit; bit >>= 1; }
            j ^= bit;
            if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
        }
        for (int len = 2; len <= m_fftSize; len <<= 1) {
            double angle = -2.0 * M_PI / len;
            for (int i = 0; i < m_fftSize; i += len) {
                for (int k = 0; k < len / 2; ++k) {
                    double wr = qCos(angle * k), wi = qSin(angle * k);
                    double tRe = re[i+k+len/2]*wr - im[i+k+len/2]*wi;
                    double tIm = re[i+k+len/2]*wi + im[i+k+len/2]*wr;
                    re[i+k+len/2] = re[i+k] - tRe;
                    im[i+k+len/2] = im[i+k] - tIm;
                    re[i+k] += tRe;
                    im[i+k] += tIm;
                }
            }
        }

        // Overlap-add with Hann window
        for (int i = 0; i < m_fftSize && start + i < totalSamples; ++i) {
            double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
            output[start + i] += re[i] / m_fftSize * w;
            windowSum[start + i] += w * w;
        }
    }

    // Normalize by window sum
    for (int i = 0; i < totalSamples; ++i)
        if (windowSum[i] > 1e-10) output[i] /= windowSum[i];

    return output;
}

/* ---- Spectral gating via STFT ---- */

QVector<double> NoiseGate9::processSpectral(const QVector<double>& input, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    if (m_fftSize == 0) return processTimeDomain(input, sampleRate);

    int n = input.size();
    int hopSize = m_fftSize / 2;

    // Estimate noise profile if not set
    if (m_noisePower.isEmpty()) estimateNoiseProfile(input.mid(0, qMin(m_fftSize * 4, n)));

    QVector<QVector<double>> mag, ph;
    computeSTFT(input, hopSize, mag, ph);

    double floorLinear = qPow(10.0, m_spectralFloorDb / 20.0);
    int halfN = m_fftSize / 2;

    // Apply spectral gating per frame per bin
    for (int f = 0; f < mag.size(); ++f) {
        for (int b = 0; b <= halfN; ++b) {
            double threshold = (b < m_noisePower.size()) ? m_noisePower[b] * 2.0 : 1e-10;
            if (mag[f][b] < threshold) {
                mag[f][b] *= floorLinear;  // Attenuate below threshold
            }
        }
    }

    QVector<double> output = inverseSTFT(mag, ph, hopSize, n);

    double elapsed = timer.elapsed();
    m_stats.frameSize = m_fftSize;
    m_stats.numFrames += mag.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(mag.size(), 0.0, elapsed);

    return output;
}

/* ---- Estimate noise profile ---- */

void NoiseGate9::estimateNoiseProfile(const QVector<double>& noiseSegment)
{
    if (noiseSegment.isEmpty() || m_fftSize == 0) return;
    int halfN = m_fftSize / 2;
    int numFrames = noiseSegment.size() / m_fftSize;
    if (numFrames < 1) numFrames = 1;

    m_noisePower.resize(halfN + 1);
    m_noisePower.fill(0.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_fftSize;
        for (int b = 0; b <= halfN; ++b) {
            double sumSq = 0.0;
            for (int i = 0; i < m_fftSize && start + i < noiseSegment.size(); ++i) {
                double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_fftSize));
                sumSq += noiseSegment[start + i] * w;
            }
            m_noisePower[b] += sumSq * sumSq;
        }
    }
    for (int b = 0; b <= halfN; ++b)
        m_noisePower[b] = qSqrt(m_noisePower[b] / numFrames);
}

/* ---- Reset ---- */

void NoiseGate9::resetStatistics()
{
    m_state = Closed;
    m_gain = 0.0;
    m_noisePower.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
