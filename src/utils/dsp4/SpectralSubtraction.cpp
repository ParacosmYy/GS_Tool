/**
 * @file SpectralSubtraction.cpp
 * @brief 谱减法降噪实现
 */

#include "SpectralSubtraction.h"
#include <QElapsedTimer>
#include <cmath>

SpectralSubtraction::SpectralSubtraction(int fftSize, int noiseFrames,
                                           QObject* parent)
    : QObject(parent)
    , m_fftSize(qMax(64, fftSize))
    , m_noiseFrames(qMax(1, noiseFrames))
    , m_timeSum(0.0)
{
}

void SpectralSubtraction::estimateNoise(const QVector<double>& noiseSamples)
{
    int halfFFT = m_fftSize / 2 + 1;
    m_noisePower.fill(0.0, halfFFT);

    int frameCount = 0;
    int hopSize = m_fftSize / 2;

    for (int pos = 0; pos + m_fftSize <= noiseSamples.size() && frameCount < m_noiseFrames;
         pos += hopSize, ++frameCount) {

        for (int k = 0; k < halfFFT; ++k) {
            double re = 0.0, im = 0.0;
            for (int n = 0; n < m_fftSize; ++n) {
                double w = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (m_fftSize - 1)));
                double angle = -2.0 * M_PI * k * n / m_fftSize;
                re += noiseSamples[pos + n] * w * std::cos(angle);
                im += noiseSamples[pos + n] * w * std::sin(angle);
            }
            m_noisePower[k] += (re * re + im * im) / m_fftSize;
        }
    }

    if (frameCount > 0) {
        for (double& v : m_noisePower)
            v /= frameCount;
    }
}

QVector<double> SpectralSubtraction::process(const QVector<double>& signal,
                                               double overSubtraction,
                                               double spectralFloor)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    int hopSize = m_fftSize / 2;
    QVector<double> output(N, 0.0);
    QVector<double> windowSum(N, 0.0);

    /* Hanning窗 */
    QVector<double> window(m_fftSize);
    for (int i = 0; i < m_fftSize; ++i)
        window[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (m_fftSize - 1)));

    for (int pos = 0; pos + m_fftSize <= N; pos += hopSize) {
        /* 提取帧 */
        QVector<double> frame(m_fftSize);
        for (int i = 0; i < m_fftSize; ++i)
            frame[i] = signal[pos + i] * window[i];

        /* 处理帧 */
        auto cleaned = processFrame(frame, overSubtraction, spectralFloor);

        /* OLA叠加 */
        for (int i = 0; i < m_fftSize; ++i) {
            output[pos + i] += cleaned[i];
            windowSum[pos + i] += window[i] * window[i];
        }

        m_stats.totalFrames++;
    }

    /* 归一化 */
    for (int i = 0; i < N; ++i) {
        if (windowSum[i] > 1e-10)
            output[i] /= windowSum[i];
    }

    m_stats.totalProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;

    emit processingCompleted(N, N);
    return output;
}

QVector<double> SpectralSubtraction::processFrame(const QVector<double>& frame,
                                                     double overSubtraction,
                                                     double spectralFloor)
{
    int halfFFT = m_fftSize / 2 + 1;
    int N = frame.size();

    /* DFT */
    QVector<double> power(halfFFT);
    for (int k = 0; k < halfFFT; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / m_fftSize;
            re += frame[n] * std::cos(angle);
            im += frame[n] * std::sin(angle);
        }
        power[k] = re * re + im * im;
    }

    /* 谱减 */
    for (int k = 0; k < halfFFT; ++k) {
        double noiseEst = (k < m_noisePower.size()) ? m_noisePower[k] : 0.0;
        double magnitude = std::sqrt(std::max(power[k] - overSubtraction * noiseEst,
                                               spectralFloor * power[k]));
        power[k] = magnitude;
    }

    /* IDFT(简化: 只重建时域) */
    QVector<double> result(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double val = 0.0;
        for (int k = 0; k < halfFFT; ++k) {
            double angle = 2.0 * M_PI * k * n / m_fftSize;
            val += std::sqrt(power[k]) * std::cos(angle);
        }
        result[n] = val * 2.0 / m_fftSize;
    }

    return result;
}

void SpectralSubtraction::setFftSize(int size)
{
    m_fftSize = qMax(64, size);
    m_noisePower.clear();
}

SpectralSubtraction::Stats SpectralSubtraction::stats() const { return m_stats; }

void SpectralSubtraction::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
