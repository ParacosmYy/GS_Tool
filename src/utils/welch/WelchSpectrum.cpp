/**
 * @file WelchSpectrum.cpp
 * @brief Welch谱估计实现
 */

#include "utils/welch/WelchSpectrum.h"

#include <QElapsedTimer>
#include <cmath>

WelchSpectrum::WelchSpectrum(QObject* parent)
    : QObject(parent), m_segmentLength(256), m_overlap(0.5),
      m_window(Hanning), m_timeSum(0.0) {}

void WelchSpectrum::configure(int segmentLength, double overlap,
                               Window window)
{
    m_segmentLength = qMax(16, segmentLength);
    m_overlap = qBound(0.0, overlap, 0.9);
    m_window = window;
}

QVector<double> WelchSpectrum::createWindow(int length, Window type) const
{
    QVector<double> w(length);
    for (int i = 0; i < length; ++i) {
        double n = static_cast<double>(i) / (length - 1);
        switch (type) {
        case Hanning:
            w[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * n));
            break;
        case Hamming:
            w[i] = 0.54 - 0.46 * std::cos(2.0 * M_PI * n);
            break;
        case Blackman:
            w[i] = 0.42 - 0.5 * std::cos(2.0 * M_PI * n) +
                    0.08 * std::cos(4.0 * M_PI * n);
            break;
        case Rectangular:
            w[i] = 1.0;
            break;
        }
    }
    return w;
}

QPair<QVector<double>, QVector<double>> WelchSpectrum::estimate(
    const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    int L = qMin(m_segmentLength, N);
    int step = qMax(1, static_cast<int>(L * (1.0 - m_overlap)));
    QVector<double> win = createWindow(L, m_window);

    double winPower = 0.0;
    for (double w : win) winPower += w * w;

    int nfft = L;
    int halfBins = nfft / 2 + 1;
    QVector<double> psd(halfBins, 0.0);
    int segCount = 0;

    for (int start = 0; start + L <= N; start += step) {
        /* 窗函数+去均值 */
        double segMean = 0.0;
        for (int i = 0; i < L; ++i)
            segMean += signal[start + i];
        segMean /= L;

        /* DFT(简化直接计算) */
        for (int k = 0; k < halfBins; ++k) {
            double real = 0.0, imag = 0.0;
            for (int i = 0; i < L; ++i) {
                double val = (signal[start + i] - segMean) * win[i];
                double angle = -2.0 * M_PI * k * i / nfft;
                real += val * std::cos(angle);
                imag += val * std::sin(angle);
            }
            psd[k] += (real * real + imag * imag);
        }
        segCount++;
    }

    /* 归一化 */
    if (segCount > 0 && winPower > 0) {
        double norm = sampleRate / (segCount * winPower * nfft);
        for (auto& v : psd) v *= norm;
    }

    /* 频率轴 */
    QVector<double> freqs(halfBins);
    for (int k = 0; k < halfBins; ++k)
        freqs[k] = static_cast<double>(k) * sampleRate / nfft;

    m_stats.totalEstimates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimates;

    double maxPSD = 0.0;
    for (double v : psd) if (v > maxPSD) maxPSD = v;
    emit estimateCompleted(halfBins, maxPSD);

    return {freqs, psd};
}

void WelchSpectrum::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
