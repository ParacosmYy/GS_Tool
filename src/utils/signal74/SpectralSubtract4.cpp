/**
 * @file SpectralSubtract4.cpp
 * @brief 谱减法降噪实现
 *
 * 实现经典的谱减法(Spectral Subtraction)降噪算法，
 * 通过估计噪声功率谱并从含噪信号中减去来降低噪声。
 */

#include "utils/signal74/SpectralSubtract4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

SpectralSubtract4::SpectralSubtract4(QObject* parent) : QObject(parent) {}

void SpectralSubtract4::setFFTSize(int n) { m_fftSize = qMax(64, n); }
void SpectralSubtract4::setOverSubtraction(double factor) { m_overSub = qBound(0.5, factor, 10.0); }
void SpectralSubtract4::setSpectralFloor(double floor) { m_floor = qBound(0.001, floor, 0.5); }
void SpectralSubtract4::setNoiseEstimate(const QVector<double>& noise) { m_noise = noise; }

/**
 * @brief 对含噪信号执行谱减法降噪
 * @param signal 含噪信号
 * @return 降噪后的信号
 */
QVector<double> SpectralSubtract4::denoise(const QVector<double>& signal) {
    QElapsedTimer timer; timer.start();

    QVector<double> output;
    const int N = signal.size();
    if (N == 0 || m_noise.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalDenoisings++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoisings;
        return output;
    }

    const int fftSize = m_fftSize;
    const int hopSize = fftSize / 2;
    const int numBins = fftSize / 2 + 1;

    QVector<double> noisePower(numBins, 0.0);
    for (int i = 0; i < numBins && i < m_noise.size(); ++i)
        noisePower[i] = m_noise[i] * m_noise[i];

    output.resize(N, 0.0);
    QVector<double> windowSum(N, 0.0);

    QVector<double> window(fftSize);
    for (int i = 0; i < fftSize; ++i)
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (fftSize - 1)));

    int frameCount = 0;
    for (int start = 0; start + fftSize <= N; start += hopSize) {
        frameCount++;
        QVector<double> frame(fftSize, 0.0);
        for (int i = 0; i < fftSize; ++i) frame[i] = signal[start + i] * window[i];

        QVector<double> magnitude(numBins, 0.0), phase(numBins, 0.0);
        for (int k = 0; k < numBins; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < fftSize; ++n) {
                double angle = 2.0 * M_PI * k * n / fftSize;
                re += frame[n] * qCos(angle);
                im -= frame[n] * qSin(angle);
            }
            magnitude[k] = qSqrt(re * re + im * im);
            phase[k] = qAtan2(im, re);
        }

        QVector<double> cleanMag(numBins, 0.0);
        for (int k = 0; k < numBins; ++k) {
            double sigPow = magnitude[k] * magnitude[k];
            double nEst = (k < noisePower.size()) ? noisePower[k] : 0.0;
            double sub = sigPow - m_overSub * nEst;
            double floorVal = m_floor * sigPow;
            cleanMag[k] = qSqrt(qMax(sub, floorVal));
        }

        QVector<double> cleanFrame(fftSize, 0.0);
        for (int n = 0; n < fftSize; ++n) {
            double val = 0.0;
            for (int k = 0; k < numBins; ++k)
                val += cleanMag[k] * qCos(2.0 * M_PI * k * n / fftSize + phase[k]);
            cleanFrame[n] = val / fftSize;
        }

        for (int i = 0; i < fftSize && start + i < N; ++i) {
            output[start + i] += cleanFrame[i] * window[i];
            windowSum[start + i] += window[i] * window[i];
        }
    }

    for (int i = 0; i < N; ++i)
        if (windowSum[i] > 1e-10) output[i] /= windowSum[i];

    double sigE = 0.0, noiseE = 0.0;
    for (int i = 0; i < N; ++i) {
        sigE += output[i] * output[i];
        double d = signal[i] - output[i];
        noiseE += d * d;
    }
    double snr = (noiseE > 1e-15) ? 10.0 * qLn(sigE / noiseE) / qLn(10.0) : 100.0;

    qint64 elapsed = timer.elapsed();
    m_stats.totalDenoisings++;
    m_stats.totalFrames += frameCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDenoisings;
    emit denoisingCompleted(frameCount, snr);
    return output;
}

void SpectralSubtract4::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }
