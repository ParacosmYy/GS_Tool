/**
 * @file SpectralGate7.cpp
 * @brief SpectralGate7 实现
 *
 * 实现谱门控：Wiener滤波估计与噪声底噪减除宽带噪声抑制。
 */

#include "utils/dsp265/SpectralGate7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralGate7::SpectralGate7(QObject *parent)
    : QObject(parent) {}

SpectralGate7::~SpectralGate7() = default;

/* ---- Configuration ---- */

void SpectralGate7::setParameters(int frameSize, int fftSize, int noiseFrames)
{
    m_frameSize = qMax(64, frameSize);
    m_fftSize = (fftSize > 0) ? qMax(m_frameSize, fftSize) : m_frameSize * 2;
    m_noiseFrames = qMax(1, noiseFrames);
    m_overlap = m_frameSize / 2;
    m_window = createHannWindow(m_frameSize);
}

/* ---- Hann window ---- */

QVector<double> SpectralGate7::createHannWindow(int size) const
{
    QVector<double> w(size);
    for (int i = 0; i < size; ++i)
        w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (size - 1)));
    return w;
}

QVector<double> SpectralGate7::applyWindow(const QVector<double>& frame) const
{
    QVector<double> out(frame.size());
    for (int i = 0; i < frame.size() && i < m_window.size(); ++i)
        out[i] = frame[i] * m_window[i];
    return out;
}

/* ---- FFT (radix-2 Cooley-Tukey) ---- */

void SpectralGate7::fft(const QVector<double>& real, const QVector<double>& imag,
                          QVector<double>& outReal, QVector<double>& outImag) const
{
    int n = real.size();
    outReal = real;
    outImag = imag;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(outReal[i], outReal[j]);
            std::swap(outImag[i], outImag[j]);
        }
    }

    // Butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = curRe * outReal[i+j+len/2] - curIm * outImag[i+j+len/2];
                double tIm = curRe * outImag[i+j+len/2] + curIm * outReal[i+j+len/2];
                outReal[i+j+len/2] = outReal[i+j] - tRe;
                outImag[i+j+len/2] = outImag[i+j] - tIm;
                outReal[i+j] += tRe;
                outImag[i+j] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

void SpectralGate7::ifft(const QVector<double>& real, const QVector<double>& imag,
                           QVector<double>& outReal, QVector<double>& outImag) const
{
    int n = real.size();
    QVector<double> conjImag(n);
    for (int i = 0; i < n; ++i) conjImag[i] = -imag[i];
    fft(real, conjImag, outReal, outImag);
    for (int i = 0; i < n; ++i) {
        outReal[i] /= n;
        outImag[i] = -outImag[i] / n;
    }
}

/* ---- Magnitude ---- */

QVector<double> SpectralGate7::magnitude(const QVector<double>& re,
                                           const QVector<double>& im) const
{
    QVector<double> mag(re.size());
    for (int i = 0; i < re.size(); ++i)
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
    return mag;
}

/* ---- Estimate noise profile ---- */

void SpectralGate7::estimateNoiseProfile(
    const QVector<QVector<double>>& noiseFrames)
{
    if (noiseFrames.isEmpty()) return;
    int bins = m_fftSize / 2 + 1;
    m_noisePower.resize(bins);
    m_noisePower.fill(0.0);

    int count = qMin(noiseFrames.size(), m_noiseFrames);
    for (int f = 0; f < count; ++f) {
        auto windowed = applyWindow(noiseFrames[f]);
        QVector<double> padded(m_fftSize, 0.0);
        for (int i = 0; i < qMin(windowed.size(), m_fftSize); ++i)
            padded[i] = windowed[i];

        QVector<double> zeroImag(m_fftSize, 0.0);
        QVector<double> specRe, specIm;
        fft(padded, zeroImag, specRe, specIm);

        for (int i = 0; i < bins; ++i) {
            double p = specRe[i] * specRe[i] + specIm[i] * specIm[i];
            m_noisePower[i] += p / count;
        }
    }

    m_noiseMagnitude.resize(bins);
    for (int i = 0; i < bins; ++i)
        m_noiseMagnitude[i] = qSqrt(m_noisePower[i]);
}

/* ---- Wiener gain ---- */

QVector<double> SpectralGate7::wienerGain(
    const QVector<double>& signalPower) const
{
    int n = qMin(signalPower.size(), m_noisePower.size());
    QVector<double> gain(n);
    for (int i = 0; i < n; ++i) {
        double snr = (signalPower[i] - m_noisePower[i]) / qMax(m_noisePower[i], 1e-10);
        // Wiener gain: max(0, 1 - 1/SNR)
        gain[i] = qMax(0.0, 1.0 - 1.0 / qMax(snr, 0.001));
        // Spectral gate threshold: suppress below noise floor
        if (signalPower[i] < m_noisePower[i] * 1.5)
            gain[i] *= 0.1;  // Hard gate below 1.5x noise
    }
    return gain;
}

/* ---- Compute SNR ---- */

double SpectralGate7::computeSnr(const QVector<double>& signal,
                                   const QVector<double>& noise) const
{
    double sigPow = 0.0, noisePow = 0.0;
    int n = qMin(signal.size(), noise.size());
    for (int i = 0; i < n; ++i) {
        sigPow += signal[i] * signal[i];
        noisePow += noise[i] * noise[i];
    }
    if (noisePow < 1e-20) return 60.0;
    return 10.0 * qLn(sigPow / noisePow) / qLn(10.0);
}

/* ---- Process single frame ---- */

QVector<double> SpectralGate7::processFrame(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    auto windowed = applyWindow(frame);
    QVector<double> padded(m_fftSize, 0.0);
    for (int i = 0; i < qMin(windowed.size(), m_fftSize); ++i)
        padded[i] = windowed[i];

    // Forward FFT
    QVector<double> zeroImag(m_fftSize, 0.0);
    QVector<double> specRe, specIm;
    fft(padded, zeroImag, specRe, specIm);

    int bins = m_fftSize / 2 + 1;

    // Compute signal power
    QVector<double> sigPow(bins);
    for (int i = 0; i < bins; ++i)
        sigPow[i] = specRe[i] * specRe[i] + specIm[i] * specIm[i];

    // Apply Wiener gain and noise subtraction
    if (!m_noisePower.isEmpty()) {
        auto gain = wienerGain(sigPow);
        for (int i = 0; i < bins; ++i) {
            specRe[i] *= gain[i];
            specIm[i] *= gain[i];
        }
        // Mirror for negative frequencies
        for (int i = bins; i < m_fftSize; ++i) {
            int mirror = m_fftSize - i;
            if (mirror < bins) {
                specRe[i] = specRe[mirror] * (i == m_fftSize - mirror ? 1.0 : 1.0);
                specIm[i] = -specIm[mirror];
            }
        }
    }

    // Inverse FFT
    QVector<double> outRe, outIm;
    ifft(specRe, specIm, outRe, outIm);

    // Extract real part up to frame size
    QVector<double> result(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        result[i] = outRe[i];

    double elapsed = timer.elapsed();
    double snr = m_noisePower.isEmpty() ? 0.0 : computeSnr(sigPow, m_noisePower);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit frameProcessed(static_cast<int>(m_stats.totalOps), snr, elapsed);
    return result;
}

/* ---- Process entire signal ---- */

QVector<double> SpectralGate7::process(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int hop = m_frameSize - m_overlap;
    int numFrames = (signal.size() - m_overlap) / hop;
    if (numFrames < 1) return signal;

    QVector<double> output(signal.size(), 0.0);
    QVector<double> windowSum(signal.size(), 0.0);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hop;
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize && start + i < signal.size(); ++i)
            frame[i] = signal[start + i];

        auto processed = processFrame(frame);
        for (int i = 0; i < m_frameSize && start + i < signal.size(); ++i) {
            output[start + i] += processed[i] * m_window[i];
            windowSum[start + i] += m_window[i] * m_window[i];
        }
    }

    // Overlap-add normalization
    for (int i = 0; i < output.size(); ++i)
        if (windowSum[i] > 1e-10) output[i] /= windowSum[i];

    double elapsed = timer.elapsed();
    m_stats.frameSize = m_frameSize;
    m_stats.fftSize = m_fftSize;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return output;
}

/* ---- Accessors ---- */

QVector<double> SpectralGate7::noiseProfile() const { return m_noiseMagnitude; }

void SpectralGate7::setNoiseProfile(const QVector<double>& profile)
{
    m_noiseMagnitude = profile;
    m_noisePower.resize(profile.size());
    for (int i = 0; i < profile.size(); ++i)
        m_noisePower[i] = profile[i] * profile[i];
}

/* ---- Reset ---- */

void SpectralGate7::resetStatistics()
{
    m_noiseMagnitude.clear();
    m_noisePower.clear();
    m_window.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
