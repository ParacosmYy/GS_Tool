/**
 * @file SpectralGate6.cpp
 * @brief SpectralGate6 实现
 *
 * 实现频谱门控：Wiener滤波估计与噪声轮廓学习自适应谱减。
 */

#include "utils/dsp251/SpectralGate6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralGate6::SpectralGate6(QObject *parent) : QObject(parent)
{
    m_noisePSD.resize(m_fftSize / 2 + 1, 0.0);
    m_noiseSum.resize(m_fftSize / 2 + 1, 0.0);
    buildWindow();
}

SpectralGate6::~SpectralGate6() = default;

/* ---- Build Hann window ---- */

void SpectralGate6::buildWindow()
{
    m_window.resize(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_fftSize - 1)));
}

/* ---- Radix-2 FFT ---- */

void SpectralGate6::radix2FFT(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]); std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double tRe = curRe * re[i+j+len/2] - curIm * im[i+j+len/2];
                double tIm = curRe * im[i+j+len/2] + curIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                double newRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
            }
        }
    }
}

/* ---- Radix-2 IFFT ---- */

void SpectralGate6::radix2IFFT(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    for (auto& v : im) v = -v;
    radix2FFT(re, im);
    for (auto& v : im) v = -v;
    for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
}

/* ---- Compute magnitude spectrum ---- */

void SpectralGate6::computeSpectrum(const QVector<double>& frame,
                                      QVector<double>& magnitude,
                                      QVector<double>& phase) const
{
    int n = m_fftSize;
    int bins = n / 2 + 1;
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < qMin(frame.size(), n); ++i)
        re[i] = frame[i] * m_window[i];

    radix2FFT(re, im);

    magnitude.resize(bins);
    phase.resize(bins);
    for (int k = 0; k < bins; ++k) {
        magnitude[k] = re[k] * re[k] + im[k] * im[k];
        phase[k] = qAtan2(im[k], re[k]);
    }
}

/* ---- Wiener gain ---- */

QVector<double> SpectralGate6::wienerGain(const QVector<double>& signalPSD) const
{
    int bins = signalPSD.size();
    QVector<double> gain(bins, 0.0);
    for (int k = 0; k < bins; ++k) {
        double noiseEst = m_noisePSD[k] * m_threshold;
        double snr = (signalPSD[k] - noiseEst) / qMax(noiseEst, 1e-10);
        // Wiener gain: max(SNR / (1 + SNR), floor)
        gain[k] = qMax(snr / (1.0 + qMax(snr, 0.0)), m_floor);
    }
    return gain;
}

/* ---- Reconstruct time-domain ---- */

QVector<double> SpectralGate6::reconstruct(const QVector<double>& magnitude,
                                             const QVector<double>& phase) const
{
    int n = m_fftSize;
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int k = 0; k < qMin(magnitude.size(), n / 2 + 1); ++k) {
        double mag = qSqrt(qMax(magnitude[k], 0.0));
        re[k] = mag * qCos(phase[k]);
        im[k] = mag * qSin(phase[k]);
    }
    // Mirror for real output
    for (int k = n / 2 + 1; k < n; ++k) {
        re[k] = re[n - k];
        im[k] = -im[n - k];
    }
    radix2IFFT(re, im);
    QVector<double> output(n);
    for (int i = 0; i < n; ++i)
        output[i] = re[i] * m_window[i];
    return output;
}

/* ---- Set FFT size ---- */

void SpectralGate6::setFFTSize(int size)
{
    int s = 256;
    while (s < size) s <<= 1;
    m_fftSize = s;
    m_noisePSD.resize(s / 2 + 1, 0.0);
    m_noiseSum.resize(s / 2 + 1, 0.0);
    buildWindow();
}

void SpectralGate6::setThreshold(double threshold) { m_threshold = qMax(0.1, threshold); }

/* ---- Learn noise profile ---- */

void SpectralGate6::learnNoiseProfile(const QVector<QVector<double>>& noiseFrames)
{
    int bins = m_fftSize / 2 + 1;
    m_noiseSum.fill(0.0);
    m_noiseCount = 0;

    for (const auto& frame : noiseFrames) {
        QVector<double> mag, ph;
        computeSpectrum(frame, mag, ph);
        for (int k = 0; k < bins; ++k)
            m_noiseSum[k] += mag[k];
        m_noiseCount++;
    }

    if (m_noiseCount > 0) {
        for (int k = 0; k < bins; ++k)
            m_noisePSD[k] = m_noiseSum[k] / m_noiseCount;
    }

    double floor = 0.0;
    for (int k = 0; k < bins; ++k) floor += m_noisePSD[k];
    m_stats.noiseFloor = floor / bins;
    m_stats.noiseFramesLearned = m_noiseCount;
    emit noiseProfileUpdated(m_noiseCount, m_stats.noiseFloor);
}

/* ---- Process single frame ---- */

QVector<double> SpectralGate6::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> mag, ph;
    computeSpectrum(frame, mag, ph);

    // Compute Wiener gain
    QVector<double> gain = wienerGain(mag);

    // Apply gain to magnitude
    QVector<double> cleanMag(mag.size());
    for (int k = 0; k < mag.size(); ++k)
        cleanMag[k] = mag[k] * gain[k];

    QVector<double> result = reconstruct(cleanMag, ph);

    double snr = 0.0;
    for (int k = 0; k < mag.size(); ++k)
        snr += cleanMag[k] / qMax(mag[k], 1e-10);
    snr /= mag.size();

    m_stats.fftSize = m_fftSize;
    m_stats.numFrames++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit frameProcessed(m_stats.numFrames - 1, snr, timer.elapsed());
    return result;
}

/* ---- Process entire signal ---- */

QVector<QVector<double>> SpectralGate6::processSignal(const QVector<QVector<double>>& frames)
{
    QVector<QVector<double>> result;
    result.reserve(frames.size());
    for (const auto& frame : frames)
        result.append(process(frame));
    return result;
}

QVector<double> SpectralGate6::noiseProfile() const { return m_noisePSD; }

/* ---- Reset ---- */

void SpectralGate6::resetStatistics()
{
    m_noisePSD.fill(0.0);
    m_noiseSum.fill(0.0);
    m_noiseCount = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
