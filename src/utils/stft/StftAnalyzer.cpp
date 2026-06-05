/**
 * @file StftAnalyzer.cpp
 * @brief 短时傅里叶变换分析器实现
 */

#include "utils/stft/StftAnalyzer.h"

#include <QtMath>
#include <QElapsedTimer>

StftAnalyzer::StftAnalyzer(QObject* parent)
    : QObject(parent), m_fftSize(256), m_hopSize(128),
      m_windowType(WindowType::Hanning), m_timeSum(0.0) {}

void StftAnalyzer::setParameters(int fftSize, int hopSize, WindowType window)
{
    m_fftSize = qMax(4, fftSize);
    m_hopSize = qMax(1, hopSize);
    m_windowType = window;
}

QList<StftAnalyzer::Frame> StftAnalyzer::analyze(const QVector<double>& data)
{
    QList<Frame> result;
    if (data.isEmpty()) return result;

    QElapsedTimer timer;
    timer.start();

    QVector<double> window = generateWindow(m_fftSize);
    int halfN = m_fftSize / 2 + 1;

    for (int start = 0; start + m_fftSize <= data.size(); start += m_hopSize) {
        QVector<double> frame(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize; ++i) {
            frame[i] = data[start + i] * window[i];
        }

        Frame f;
        f.timeIndex = start / m_hopSize;
        computeDft(frame, f.magnitude, f.phase);
        result.append(f);
    }

    m_stats.totalAnalyses++;
    m_stats.totalFramesComputed += result.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / qMax(m_stats.totalAnalyses, 1ULL);

    emit analysisCompleted(result.size(), m_fftSize);
    return result;
}

QVector<double> StftAnalyzer::synthesize(const QList<Frame>& frames,
                                          int outputLength)
{
    QVector<double> output(outputLength, 0.0);
    QVector<double> winSum(outputLength, 0.0);
    QVector<double> window = generateWindow(m_fftSize);

    for (const auto& f : frames) {
        QVector<double> timeDomain = computeIdft(f.magnitude, f.phase);
        int start = f.timeIndex * m_hopSize;

        for (int i = 0; i < m_fftSize && (start + i) < outputLength; ++i) {
            output[start + i] += timeDomain[i] * window[i];
            winSum[start + i] += window[i] * window[i];
        }
    }

    for (int i = 0; i < outputLength; ++i) {
        if (winSum[i] > 1e-10) output[i] /= winSum[i];
    }
    return output;
}

QVector<double> StftAnalyzer::generateWindow(int length) const
{
    QVector<double> w(length, 1.0);
    for (int i = 0; i < length; ++i) {
        switch (m_windowType) {
        case WindowType::Rectangular: w[i] = 1.0; break;
        case WindowType::Hamming:
            w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (length - 1)); break;
        case WindowType::Hanning:
            w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (length - 1))); break;
        case WindowType::Blackman:
            w[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (length - 1))
                  + 0.08 * qCos(4.0 * M_PI * i / (length - 1)); break;
        }
    }
    return w;
}

void StftAnalyzer::computeDft(const QVector<double>& frame,
                               QVector<double>& magnitude,
                               QVector<double>& phase) const
{
    int N = frame.size();
    int halfN = N / 2 + 1;
    magnitude.resize(halfN);
    phase.resize(halfN);

    for (int k = 0; k < halfN; ++k) {
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            real += frame[n] * qCos(angle);
            imag += frame[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(real * real + imag * imag) / N;
        phase[k] = qAtan2(imag, real);
    }
}

QVector<double> StftAnalyzer::computeIdft(const QVector<double>& magnitude,
                                           const QVector<double>& phase) const
{
    int halfN = magnitude.size();
    int N = (halfN - 1) * 2;
    QVector<double> output(N, 0.0);

    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < halfN; ++k) {
            double angle = 2.0 * M_PI * k * n / N;
            sum += magnitude[k] * qCos(angle + phase[k]);
        }
        /* 对称频率 */
        for (int k = 1; k < halfN - 1; ++k) {
            double angle = 2.0 * M_PI * (N - k) * n / N;
            sum += magnitude[k] * qCos(angle - phase[k]);
        }
        output[n] = sum;
    }
    return output;
}

void StftAnalyzer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
